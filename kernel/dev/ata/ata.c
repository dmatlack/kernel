/**
 * @file dev/ata/ata.c
 *
 * @brief Advanced Technology Attachment (ATA)
 *
 * @author David Matlack
 */
#include <dev/ata.h>

#include <kernel.h>
#include <errno.h>
#include <debug.h>

#include <arch/x86/io.h>

enum ata_drive_type get_drive_type(uint8_t clo, uint8_t chi) {
  if (clo == 0x14 && chi == 0xEB) return ATA_PATAPI;
  if (clo == 0x69 && chi == 0x96) return ATA_SATAPI;
  if (clo == 0x00 && chi == 0x00) return ATA_PATA;
  if (clo == 0x3c && chi == 0xc3) return ATA_SATA;
  return ATA_UNKNOWN;
}
bool does_ata_bus_exist(int cmd_block) {
  return inb(cmd_block + ATA_CMD_STATUS) != 0xff;
}

#define RETURN_ERROR_IF_TIMEOUT(_count, _timeout, _culprit_string) \
  do { \
    if ((_count) == (_timeout)) { \
      ERROR("Timed out waiting for %s!", _culprit_string); \
      return ETIMEDOUT; \
    } \
  } while (0)

/**
 * @brief Issue the ATA IDENTIFY command to a drive in order to get some
 * information about it.
 *
 * For more information about WTF the IDENTIFY command returns, see section
 * 8.15 of http://www.t13.org/documents/UploadedDocuments/project/d1410r3b-ATA-ATAPI-6.pdf
 *
 * @return 0 if the drive exists and reports data back via IDENTIFY, non-0
 * if the drive does not exist.
 */
int ata_identify(struct ata_drive *drive) {
  uint8_t idstatus, clo, chi;
  const int timeout = 0x100000;
  int i;

  TRACE("drive=%p", drive);

  /*
   * Select the drive
   */
  outb(drive->bus->cmd_block + ATA_CMD_DRIVE, drive->select);

  /*
   * Set all the info registers (sector count, cylinders lo/hi, etc... to 0
   */ 
  outb(drive->bus->cmd_block + ATA_CMD_SECTOR_COUNT, 0);
  outb(drive->bus->cmd_block + ATA_CMD_SECTOR_NUM,   0);
  outb(drive->bus->cmd_block + ATA_CMD_CYLINDER_LO,  0);
  outb(drive->bus->cmd_block + ATA_CMD_CYLINDER_HI,  0);

  /*
   * IDENTIFY yourself, drive!
   */
  outb(drive->bus->cmd_block + ATA_CMD_COMMAND, ATA_IDENTIFY);
  idstatus = inb(drive->bus->cmd_block + ATA_CMD_STATUS);

  if (0 == idstatus) {
    return ENODEV;
  }

  /*
   * The drive exists! Poll the BSY bit until it is cleared.
   */
  for (i = 0; i < timeout; i++) {
    idstatus = inb(drive->bus->cmd_block + ATA_CMD_STATUS);
    if (idstatus & ATA_ERR) break;
    if (!(idstatus & ATA_BSY)) break;
  }
  RETURN_ERROR_IF_TIMEOUT(i, timeout, "ATA_BSY");

  drive->exists = true;

  /*
   * Get the drive type based on the command block signature
   */
  clo = inb(drive->bus->cmd_block + ATA_CMD_CYLINDER_LO);
  chi = inb(drive->bus->cmd_block + ATA_CMD_CYLINDER_HI);
  drive->type = get_drive_type(clo, chi);

  /*
   * Continue polling until DRQ (ready to transfer data) is set or
   * ERR (error) is set.
   */
  for (i = 0; i < timeout; i++) {
    idstatus = inb(drive->bus->cmd_block + ATA_CMD_STATUS);
    if ((idstatus & ATA_DRQ) || (idstatus & ATA_ERR)) break;
  }
  RETURN_ERROR_IF_TIMEOUT(i, timeout, "ATA_DRQ");
  
  if (idstatus & ATA_ERR) {
    WARN("Error occured while waiting for ATA_DRQ after IDENTIFY.");
    WARN("#");
    WARN("#");
    WARN("# TODO: parse the error code to figure out what's actually going on!");
    WARN("#");
    WARN("#");
    return EGENERIC;
  }

  /*
   * If all succeeded, we read in the 256 byte IDENTIFY data
   */
  for (i = 0; i < 256; i++) {
    drive->identify[i] = inw(drive->bus->cmd_block + ATA_CMD_DATA);
  }
  return 0;
}

/**
 * @brief Allocate and initialize a struct ata_drive.
 *
 * (The "initialization" here is just to set some default values and init
 * the <list.h> elements of the struct.)
 */
int ata_drive_create(struct ata_drive **drivep) {
  struct ata_drive *drive;

  *drivep = drive = kmalloc(sizeof(struct ata_drive));
  if (NULL == drive) {
    return ENOMEM;
  }

  drive->bus = NULL;
  drive->type = ATA_UNKNOWN;
  drive->exists = false;

  list_elem_init(drive, ata_bus_link);

  return 0;
}

/**
 * @brief Create a new ata_drive struct and add it to the provided ata_bus
 * struct.
 *
 * @return non-0 only on UNRECOVERABLE ERROR (e.g. out of memory). this
 * function will return 0 even if the device does not exist. instead,
 * that will be indicated in the ata_device struct.
 */
int ata_bus_add_drive(struct ata_bus *bus, uint8_t drive_select) {
  struct ata_drive *drive;
  int ret, identify_error;

  if (0 != (ret = ata_drive_create(&drive))) {
    return ret;
  }

  drive->select = drive_select;
  drive->bus = bus;
  drive->exists = false;
  
  identify_error = ata_identify(drive);

  if (0 != identify_error) {
    WARN("Unable to IDENTIFY drive 0x%02x of bus 0x%03x: %s",
         drive->select, bus->cmd_block, strerr(identify_error));
  }
  else {
    DEBUG("ATA IDENTIFY: cmd_block=0x%03x, drive=0x%02x, drive->type=%s",
          bus->cmd_block, drive->select, drive_type_string(drive->type));
    DEBUG("identify[80] = 0x%04x (ATA version supporting)", drive->identify[80]);
  }

  list_insert_tail(&bus->drives, drive, ata_bus_link);

  return 0;
}

/**
 * @brief Probe an ATA bus for devices.
 *
 * This function is intended to be called by the IDE code. The IDE code knows
 * where the ATA I/O ports are (<cmd_block> and <ctl_block>). The IDE is 
 * expected to pass these values in for each ATA bus it knows about (2 in the
 * case of PIIX), and this function will probe the bus for devices.
 *
 * Upon successful compeletion, the <busp> parameter will reference a populated
 * ata_bus struct which contains a list of ATA devices which may be useful to
 * the caller.
 *
 * @return
 *    non-0 error if: out of memory (can't kmalloc)
 *
 *    0 success: otherwise (even if there is issues with the drives, we will
 *      still return success. the caller is expected to look at the ata_drive's
 *      and make sure they're all ok).
 */
int ata_new_bus(struct ata_bus *bus, unsigned cmd_block, unsigned ctl_block) {
  int ret;

  TRACE("bus=%p, cmd_block=0x%03x, ctl_block=0x%03x", bus, cmd_block, ctl_block);

  if (!does_ata_bus_exist(cmd_block)) {
    bus->exists = false;
    return 0;
  }
  bus->exists = true;

  bus->cmd_block = cmd_block;
  bus->ctl_block = ctl_block;

  list_init(&bus->drives);

  if (0 != (ret = ata_bus_add_drive(bus, ATA_SELECT_MASTER))) {
    goto free_and_return;
  }
  if (0 != (ret = ata_bus_add_drive(bus, ATA_SELECT_SLAVE))) {
    goto free_and_return;
  }

  return 0;

free_and_return:
  kfree(bus, sizeof(struct ata_bus));
  return ret;
}