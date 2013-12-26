/**
 * @file dev/pci.c
 *
 * @brief Peripheral Component Interconnect
 */
#include <dev/pci.h>
#include <string.h>
#include <stdint.h>
#include <debug.h>
#include <errno.h>
#include <kernel.h>
#include <arch/x86/io.h>
#include <types.h>
#include <stddef.h>

struct pci_bus *__pci_root;

pci_device_list_t __pci_devices;
pci_device_driver_list_t __pci_drivers;

extern struct pci_device_driver __piix_ide_driver;

/*
 * The CONFIG_ADDRESS register has the following structure:
 *
 * Bit   :  31      | 30-24    | 23-16 | 15-11  | 10-8     | 7-2      | 1-0
 * Value :  enable  | reserved | bus   | device | function | register | 00
 */
#define PCI_CONFIG(bus, device, func, offset) \
  ((1 << 31)        | \
   ((bus) << 16)    | \
   ((device) << 11) | \
   ((func) << 8)    | \
   ((offset) & (MASK(6) << 2)))

uint32_t pci_config_ind(struct pci_device *d, int offset) {
  uint32_t config;

  config = PCI_CONFIG(d->bus, d->device, d->func, offset);
  outd(CONFIG_ADDRESS, config);

  return ind(CONFIG_DATA);
}

uint16_t pci_config_inw(struct pci_device *d, int offset) {
  uint32_t data;
  data = pci_config_ind(d, offset);
  return (data >> ((offset % 4) * 8)) & 0xFFFF;
}

uint8_t pci_config_inb(struct pci_device *d, int offset) {
  uint32_t data;
  data = pci_config_ind(d, offset);
  return (data >> ((offset % 4) * 8)) & 0xFF;
}

void pci_config_outd(struct pci_device *d, int offset, uint32_t data) {
  uint32_t config;

  config = PCI_CONFIG(d->bus, d->device, d->func, offset);
  outd(CONFIG_ADDRESS, config);
  outd(CONFIG_DATA, data);
}

void pci_config_outw(struct pci_device *d, int offset, uint16_t data) {
  uint32_t dword;

  dword = pci_config_ind(d, offset);
  set_byte(&dword, offset % 4, data & 0xFF);
  set_byte(&dword, (offset+1) % 4, (data >> 8) & 0xFF);
  pci_config_outd(d, offset, dword);
}

void pci_config_outb(struct pci_device *d, int offset, uint8_t data) {
  uint32_t dword;

  dword = pci_config_ind(d, offset);
  set_byte(&dword, offset % 4, data & 0xFF);
  pci_config_outd(d, offset, dword);
}

/**
 * @brief Return true if the "device" identified by the <bus>/<device> tuple
 * is a multifunction device.
 */
bool is_multifunc_device(int bus, int device) {
  struct pci_device d;
  d.bus = bus;
  d.device = device;
  d.func = 0;

  return (pci_config_inb(&d, PCI_HEADER_TYPE) & 0x80) != 0;
}

/**
 * @brief Return true if a device, identified by the given <bus>, <device>,
 * and <func>, exists.
 */
bool device_exists(int bus, int device, int func) {
  struct pci_device d;
  d.bus = bus;
  d.device = device;
  d.func = func;

  return pci_config_inw(&d, PCI_VENDOR_ID) != 0xFFFF;
}

void pci_device_config_readall(struct pci_device *d) {

  d->device_id       = pci_config_inw(d, PCI_DEVICE_ID);
  d->vendor_id       = pci_config_inw(d, PCI_VENDOR_ID);
  d->status          = pci_config_inw(d, PCI_STATUS);
  d->command         = pci_config_inw(d, PCI_COMMAND);
  d->classcode       = pci_config_inb(d, PCI_CLASSCODE);
  d->subclass        = pci_config_inb(d, PCI_SUBCLASS);
  d->progif          = pci_config_inb(d, PCI_PROGIF);
  d->revision_id     = pci_config_inb(d, PCI_REVISION_ID);
  d->bist            = pci_config_inb(d, PCI_BIST);
  d->header_type     = pci_config_inb(d, PCI_HEADER_TYPE);
  d->latency_timer   = pci_config_inb(d, PCI_LATENCY_TIMER);
  d->cache_line_size = pci_config_inb(d, PCI_CACHE_LINE_SIZE);

  if (d->header_type == 0x00) {
    d->bar0                = pci_config_ind(d, PCI_BAR0);
    d->bar1                = pci_config_ind(d, PCI_BAR1);
    d->bar2                = pci_config_ind(d, PCI_BAR2);
    d->bar3                = pci_config_ind(d, PCI_BAR3);
    d->bar4                = pci_config_ind(d, PCI_BAR4);
    d->bar5                = pci_config_ind(d, PCI_BAR5);
    d->cardbus_cis_pointer = pci_config_ind(d, PCI_CARDBUS_CIS_POINTER);
    d->subsystem_id        = pci_config_inw(d, PCI_SUBSYSTEM_ID);
    d->subsystem_vendor_id = pci_config_inw(d, PCI_SUBSYSTEM_VENDOR_ID);
    d->expansion_rom       = pci_config_ind(d, PCI_EXPANSION_ROM);
    d->capabilities        = pci_config_inb(d, PCI_CAPABILITIES);
    d->max_latency         = pci_config_inb(d, PCI_MAX_LATENCY);
    d->min_grant           = pci_config_inb(d, PCI_MIN_GRANT);
    d->interrupt_pin       = pci_config_inb(d, PCI_INTERRUPT_PIN);
    d->interrupt_line      = pci_config_inb(d, PCI_INTERRUPT_LINE);
  }

  d->vendor_desc = pci_lookup_vendor(d->vendor_id);
  d->device_desc = pci_lookup_device(d->vendor_id, d->device_id);
  d->classcode_desc = pci_lookup_classcode(d->classcode);
  d->subclass_desc = pci_lookup_subclass(d->classcode, d->subclass, d->progif);
}

/**
 * @brief Create a pci_device struct from the device identified from the tuple
 * (bus, device, func).
 */
int pci_device_create(struct pci_device **dp, int bus, int device, int func) {
  struct pci_device *d;

  *dp = d = kmalloc(sizeof(struct pci_device));
  if (NULL == d) {
    return ENOMEM;
  }
  memset(d, 0, sizeof(struct pci_device));

  list_elem_init(d, global_link);
  list_elem_init(d, bus_link);
  
  d->num_drivers = 0;
  d->bus = bus;
  d->device = device;
  d->func = func;

  pci_device_config_readall(d);

  return 0;
}

/**
 * @brief Scan the provided bus for devices, adding those devices to the global
 * list of pci devices as well as the bus' list of devices. If a PCI-PCI bridge
 * device is found, recursively scan the downstream bus.
 */
int pci_scan_bus(struct pci_bus *b) {
  int ret, device, func;

  for (device = 0; device < 32; device++) {
    for (func = 0; func < 8; func++) {
      struct pci_device *d;

      if (!device_exists(b->bus, device, func)) continue;
      
      if (0 != (ret = pci_device_create(&d, b->bus, device, func))) {
        return ret;
      }

      list_insert_tail(&__pci_devices, d, global_link);
      list_insert_tail(&b->devices, d, bus_link);

      /*
       * If this device is a PCI-PCI bridge, then search the downstream bus
       */
      if (d->classcode == 0x06 && d->subclass == 0x04) {
        struct pci_bus *sb;
       
        sb = kmalloc(sizeof(struct pci_bus));
        if (NULL == b) {
          return ENOMEM;
        }

        list_init(&sb->devices);
        list_init(&sb->buses);
        list_elem_init(sb, bus_link);
        sb->bus = pci_config_inb(d, 0x19);
        sb->self = d;
    
        INFO("PCI-PCI Bridge found. Secondary Bus: %d", sb->bus);

        pci_scan_bus(sb);
      }

      if (func == 0 && !is_multifunc_device(b->bus, device)) break;
    }
  }

  return 0;
}

void pci_print_device(struct pci_device *d) {
  /*
   * Follow linux's lspci command format of <bus>:<device>.<func>
   */
  INFO("PCI DEVICE (%p) %02x:%02x.%02x\n"
    "vendor id:           0x%04x   %s\n"
    "device id:           0x%04x   %s\n"
    "classcode:           0x%02x     %s\n"
    "subclass:            0x%02x     %s\n"
    "status:              0x%04x\n"
    "command:             0x%04x\n"
    "progif:              0x%02x\n"
    "revision id:         0x%02x\n"
    "bist:                0x%02x\n"
    "header type:         0x%02x\n"
    "latency timer:       0x%02x\n"
    "cache line size:     0x%02x\n"
    "\n"
    "bar0:                0x%08x\n"
    "bar1:                0x%08x\n"
    "bar2:                0x%08x\n"
    "bar3:                0x%08x\n"
    "bar4:                0x%08x\n"
    "bar5:                0x%08x\n"
    "cardbus cis ptr:     0x%08x\n"
    "subsystem id:        0x%04x\n"
    "subsystem vendor id: 0x%04x\n"
    "expansion rom:       0x%08x\n"
    "capabilities:        0x%02x\n"
    "max latency:         0x%02x\n"
    "min grant:           0x%02x\n"
    "interrupt pin:       0x%02x\n"
    "interrupt line:      0x%02x\n"
    "\n",
    d, d->bus, d->device, d->func,
    d->vendor_id, d->vendor_desc,
    d->device_id, d->device_desc,
    d->classcode, d->classcode_desc,
    d->subclass, d->subclass_desc,
    d->status,
    d->command,
    d->progif,
    d->revision_id,
    d->bist,
    d->header_type,
    d->latency_timer,
    d->cache_line_size,
    d->bar0,
    d->bar1,
    d->bar2,
    d->bar3,
    d->bar4,
    d->bar5,
    d->cardbus_cis_pointer,
    d->subsystem_id,
    d->subsystem_vendor_id,
    d->expansion_rom,
    d->capabilities,
    d->max_latency,
    d->min_grant,
    d->interrupt_pin,
    d->interrupt_line
  );
}

void __lspci(struct pci_bus *root, int depth) {
  struct pci_device *d;
  struct pci_bus *sb;

  INFO("%*sBus: %d", depth, "", root->bus);
  list_foreach(d, &root->devices, bus_link) {

    pci_print_device(d);
    kprintf("%02x:%02x.%02x %s, %s, %s, %s\n", 
        d->bus, d->device, d->func,
        d->vendor_desc, d->device_desc, d->classcode_desc, d->subclass_desc);

    list_foreach(sb, &root->buses, bus_link) {
      if (sb->self == d) __lspci(sb, depth + 1);
    }
  }
}

void lspci(void) {
  __lspci(__pci_root, 0);
}

/**
 * @brief Return true if the given device <d> matches the pci_device_id
 * struct <id>.
 */
bool pci_device_match(struct pci_device_id *id, struct pci_device *d) {
  if (id->vendor_id != d->vendor_id && id->vendor_id != PCI_VENDOR_ANY)
    return false;
  if (id->device_id != d->device_id && id->device_id != PCI_DEVICE_ANY)
    return false;
  if (id->classcode != d->classcode && id->classcode != PCI_CLASSCODE_ANY)
    return false;
  if (id->subclass != d->subclass && id->subclass != PCI_SUBCLASS_ANY)
    return false;
  return true;
}

void pci_device_add_driver(struct pci_device *d, 
                           struct pci_device_driver *driver) {
  int ret;

  if (d->num_drivers == PCI_DEVICE_MAX_DRIVERS) {
    WARN("Attempted to register more than %d drivers for device "
         "%04x:%04x:%02x.%02x", 
         PCI_DEVICE_MAX_DRIVERS, d->vendor_id, d->device_id, d->classcode,
         d->subclass);
    return;
  }

  if (0 != (ret = driver->new_device(d))) {
    WARN("Failed to add device %04x:%04x:%02x.%02x to driver %s: %s",
         d->vendor_id, d->device_id, d->classcode, d->subclass,
         driver->name, strerr(ret));
    return;
  }

  d->drivers[d->num_drivers++] = driver;
}

void pci_register_driver(struct pci_device_driver *driver) {
  struct pci_device *d;
  int ret;

  if (0 != (ret = driver->init())) {
    WARN("Failed to initalize the %s driver system: %s", driver->name,
         strerr(ret));
    return;
  }

  list_elem_init(driver, pci_link);
  list_insert_tail(&__pci_drivers, driver, pci_link);

  /*
   * invoke the driver on each matching device
   */
  list_foreach(d, &__pci_devices, global_link) {
    if (pci_device_match(&driver->id, d)) {
      pci_device_add_driver(d, driver);
    }
  }
}

/**
 * @brief Initialize the PCI subsystem. This initialization will build
 * the PCI bus tree, and call driver initializers on all devices that have
 * matching drivers.
 */
int pci_init(void) {
  int ret;

  TRACE();

  list_init(&__pci_devices);
  list_init(&__pci_devices);

  __pci_root = kmalloc(sizeof(struct pci_bus));
  if (NULL == __pci_root) {
    return ENOMEM;
  }
  list_init(&__pci_root->devices);
  list_init(&__pci_root->buses);
  list_elem_init(__pci_root, bus_link);
  __pci_root->bus = 0;
  __pci_root->self = NULL;

  /*
   * construct the pci device tree
   */
  if (0 != (ret = pci_scan_bus(__pci_root))) {
    return ret;
  }

  lspci();

  /*
   * register the default drivers
   */
  pci_register_driver(&__piix_ide_driver);
  //TODO add more drivers ...

  return 0;
}