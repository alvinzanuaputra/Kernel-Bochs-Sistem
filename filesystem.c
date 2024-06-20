#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void fsInit() {
  struct map_fs map_fs_buf;
  int i = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  for (i = 0; i < 16; i++) map_fs_buf.is_used[i] = true;
  for (i = 256; i < 512; i++) map_fs_buf.is_used[i] = true;
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
}

// TODO: 2. Implement fsRead function (SUDAH)
void fsRead(struct file_metadata* metadata, enum fs_return* status) {
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
  int i, j;

  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // Find the file node
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
        strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name)) {
      if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
        *status = FS_R_TYPE_IS_DIRECTORY;
        return;
      }

      // Read file sectors
      for (j = 0; j < FS_MAX_SECTOR && data_fs_buf.datas[node_fs_buf.nodes[i].data_index].sectors[j] != 0; j++) {
        readSector(metadata->buffer + (j * SECTOR_SIZE), data_fs_buf.datas[node_fs_buf.nodes[i].data_index].sectors[j]);
      }

      metadata->filesize = j * SECTOR_SIZE;
      *status = FS_SUCCESS;
      return;
    }
  }

  *status = FS_R_NODE_NOT_FOUND;
}


// TODO: 3. Implement fsWrite function (SUDAH)
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
  int i, j, k, free_node_index = -1, free_data_index = -1;
  bool found = false;

// baca sektor jika digunakan user
  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == 0x00 && node_fs_buf.nodes[i].data_index == 0x00) {
      free_node_index = i;
      break;
    }
  }

  if (free_node_index == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }
  for (i = 0; i < FS_MAX_DATA; i++) {
    if (map_fs_buf.is_used[i] == false) {
      free_data_index = i;
      break;
    }
  }

  if (free_data_index == -1) {
    *status = FS_W_NO_FREE_DATA;
    return;
  }


  for (j = 0; j < FS_MAX_SECTOR && j * SECTOR_SIZE < metadata->filesize; j++) {
    for (k = 0; k < SECTOR_SIZE; k++) {
      data_fs_buf.datas[free_data_index].sectors[j] = j + 1;
      writeSector(metadata->buffer + (j * SECTOR_SIZE), data_fs_buf.datas[free_data_index].sectors[j]);
    }
  }

  node_fs_buf.nodes[free_node_index].parent_index = metadata->parent_index;
  node_fs_buf.nodes[free_node_index].data_index = free_data_index;
  strcpy(node_fs_buf.nodes[free_node_index].node_name, metadata->node_name);

  // Tandai sektor jika digunakan user
  map_fs_buf.is_used[free_data_index] = true;

  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  *status = FS_SUCCESS;
}
