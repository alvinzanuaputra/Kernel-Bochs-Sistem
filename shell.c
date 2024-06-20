#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell()
{
  char buf[64];
  char cmd[64];
  char arg[2][64];
  byte cwd = FS_NODE_P_ROOT;

  while (true)
  {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd"))
      cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls"))
      ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv"))
      mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp"))
      cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat"))
      cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir"))
      mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear"))
      clearScreen();
    else
      printString("Invalid command\n");
  }
}

// Untuk menjalankan program :

// make build run
// matiin bochs

// lanjut (kalau udah generate test=1 mau pindah yang lain,
// jalankan make build run dulu)

// pilih satu aja :
// make generate test=1
// make generate test=2
// make generate test=3
// make generate test=4
// make run

// Untuk menjalankan program :

// make build run
// matiin bochs

// lanjut (kalau udah generate test=1 mau pindah yang lain,
// jalankan make build run dulu)

// pilih satu aja :
// make generate test=1
// make generate test=2
// make generate test=3
// make generate test=4
// make run

// TODO: 4. Implement printCWD function (SUDAH)
void printCWD(byte cwd)
{
  struct node_fs node_fs_buf;
  int i;

  // Read filesystem node sectors
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // Jika direktori saat ini adalah root, cukup cetak "/"
  if (cwd == FS_NODE_P_ROOT)
  {
    printString("/");
    return;
  }

  // Temukan direktori saat ini dan cetak parent directory terlebih dahulu
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (i == cwd)
    {
      // Cetak parent directory terlebih dahulu
      printCWD(node_fs_buf.nodes[i].parent_index);
      printString(node_fs_buf.nodes[i].node_name);
      printString("/");
      return;
    }
  }
}

// FUNGSI (yang tidak menampilkan direktori parent)
// void printCWD(byte cwd)
// {
//   struct node_fs node_fs_buf;
//   int i;

//   // Read filesystem node sectors
//   readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
//   readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
//   for (i = 0; i < FS_MAX_NODE; i++)
//   {
//     if (i == cwd)
//     {
//       printString(node_fs_buf.nodes[i].node_name);
//       return;
//     }
//   }
//   printString("/");
// }

// TODO: 5. Implement parseCommand function (SUDAH)
void parseCommand(char *buf, char *cmd, char arg[2][64])
{
  int i = 0, j = 0, k = 0;
  while (buf[i] != ' ' && buf[i] != '\0')
  {
    cmd[j++] = buf[i++];
  }
  cmd[j] = '\0';
  if (buf[i] == ' ')
    i++;
  while (buf[i] != ' ' && buf[i] != '\0')
  {
    arg[0][k++] = buf[i++];
  }
  arg[0][k] = '\0';
  if (buf[i] == ' ')
    i++;
  k = 0;
  while (buf[i] != ' ' && buf[i] != '\0')
  {
    arg[1][k++] = buf[i++];
  }
  arg[1][k] = '\0';
}

// TODO: 6. Implement cd function (SUDAH)
void cd(byte *cwd, char *dirname)
{
  struct node_fs node_fs_buf;
  int i;
  bool found = false;

  // if (dirname[0] == '\0')
  // {
  // Tidak melakukan apapun jika filename kosong atau cetak file input nya ngga ada
  //   printString("Invalid input dirname\n");
  //   return;
  // }

  // Jika dirname adalah "." maka tetap di direktori saat ini
  if (strcmp(dirname, ".") == true)
    return;

  // Jika dirname adalah ".." maka pindah ke parent directory
  if (strcmp(dirname, "..") == true)
  {
    // Jika sudah di root, tetap di root
    if (*cwd == FS_NODE_P_ROOT)
      return;

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

    *cwd = node_fs_buf.nodes[*cwd].parent_index;
    return;
  }

  // Jika dirname adalah "/" maka pindah ke root directory
  if (strcmp(dirname, "/") == true)
  {
    *cwd = FS_NODE_P_ROOT;
    printString("Successfully in root directory\n");
    return;
  }

  // Membaca node sector dari filesystem
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // Mencari direktori dengan nama dirname di dalam direktori saat ini
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == *cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, dirname) == true)
    {
      if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR)
      {
        *cwd = i;
        found = true;
        break;
      }
    }
  }

  if (!found)
  {
    printString("Directory not found\n");
  }
}

// TODO: 7. Implement ls function (SUDAH)
void ls(byte cwd, char *dirname)
{
  struct node_fs node_fs_buf;
  byte folder_target_from_index = cwd;
  int N;
  bool folder_ditemukan = false;

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  if (dirname[0] != '\0' && strcmp(dirname, ".") != true)
  {
    for (N = 0; N < FS_MAX_NODE; N++)
    {
      if (node_fs_buf.nodes[N].parent_index == cwd &&
          strcmp(node_fs_buf.nodes[N].node_name, dirname))
      {
        if (node_fs_buf.nodes[N].data_index == FS_NODE_D_DIR)
        {
          folder_target_from_index = N;
          folder_ditemukan = true;
          break;
        }
      }
    }

    // (ini buat ls folder jika emang gaada folder yang mau dituju misal ls dir7 kalau ngga ada ya directory not found)
    if (!folder_ditemukan)
    {
      printString("The selected directory doesn`t exist\n");
      return;
    }
  }

  for (N = 0; N < FS_MAX_NODE; N++)
  {
    if (node_fs_buf.nodes[N].parent_index == folder_target_from_index)
    {
      printString(node_fs_buf.nodes[N].node_name);
      printString(" ");
    }
  }
  printString("\n");
}

// TODO: 8. Implement mv function (SUDAH)
void mv(byte cwd, char *src, char *dst)
{
  struct node_fs node_fs_buf;
  int src_index = -1, dst_index = -1;
  byte target_parent_index = cwd;
  int i;

  if (dst[0] == '\0')
  {
    printString("Invalid file to move\n");
    return; // Tidak melakukan apapun jika filename kosong atau cetak file input nya ngga ada
  }

  // Baca node sector
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // Cari node sumber
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, src) == true)
    {
      src_index = i;
      break;
    }
  }

  if (src_index == -1)
  {
    printString("Source file not found\n");
    return;
  }

  // Memastikan sumber bukan direktori
  if (node_fs_buf.nodes[src_index].data_index == FS_NODE_D_DIR)
  {
    printString("Cannot move directory\n");
    return;
  }

  // Jika tujuan adalah parent directory
  if (strcmp(dst, "../") == 0 && cwd != FS_NODE_P_ROOT)
  {
    target_parent_index = node_fs_buf.nodes[cwd].parent_index;
    strcpy(node_fs_buf.nodes[src_index].node_name, src);
  }
  // Tujuan ke direktori lain atau dengan nama file baru
  else
  {
    // Jika tujuan termasuk nama direktori
    char *dir_name = dst;
    char *file_name = dst;
    bool has_dir = false;

    // Cek apakah ada '/'
    for (i = 0; dst[i] != '\0'; i++)
    {
      if (dst[i] == '/')
      {
        has_dir = true;
        break;
      }
    }

    if (has_dir)
    {
      // Pisahkan direktori dan nama file
      dir_name = dst;
      file_name = dst + i + 1;
      dst[i] = '\0';

      // Cari direktori tujuan
      for (i = 0; i < FS_MAX_NODE; i++)
      {
        if (node_fs_buf.nodes[i].parent_index == cwd &&
            strcmp(node_fs_buf.nodes[i].node_name, dir_name) == true)
        {
          target_parent_index = i;
          break;
        }
      }

      if (target_parent_index == cwd)
      {
        printString("Target directory not found\n");
        return;
      }

      // Atur nama file baru
      strcpy(node_fs_buf.nodes[src_index].node_name, file_name);
    }
    else
    {
      // Jika tujuan hanya nama file baru
      strcpy(node_fs_buf.nodes[src_index].node_name, dst);
    }
  }

  // Pindahkan file (parent artinya sebelumnya)
  node_fs_buf.nodes[src_index].parent_index = target_parent_index;

  // Tulis kembali node buffer ke disk
  writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  printString("File moved successfully\n");
}

// // TODO: . ini buat RENAME FILE bisa
// void mv(byte cwd, char *src, char *dst) {
//     struct node_fs node_fs_buf;
//     int src_index = -1, dst_index = -1;
//     int i;

//     // Baca node sector
//     readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
//     readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

//     // Cari node sumber
//     for (i = 0; i < FS_MAX_NODE; i++) {
//         if (node_fs_buf.nodes[i].parent_index == cwd &&
//             strcmp(node_fs_buf.nodes[i].node_name, src) == true) {
//             src_index = i;
//             break;
//         }
//     }

//     if (src_index == -1) {
//         printString("Source file not found\n");
//         return;
//     }

//     // Memastikan sumber bukan direktori
//     if (node_fs_buf.nodes[src_index].data_index == FS_NODE_D_DIR) {
//         printString("Cannot rename file or directory\n");
//         return;
//     }

//     // Jika tujuan adalah root directory
//     if (dst[0] == '/' && dst[1] == '\0') {
//         node_fs_buf.nodes[src_index].parent_index = FS_NODE_P_ROOT;
//         strcpy(node_fs_buf.nodes[src_index].node_name, src);
//     }
//     // Jika tujuan adalah parent directory
//     else if (dst[0] == '.' && dst[1] == '.' && dst[2] == '/' && dst[3] == '\0') {
//         node_fs_buf.nodes[src_index].parent_index = node_fs_buf.nodes[cwd].parent_index;
//         strcpy(node_fs_buf.nodes[src_index].node_name, src);
//     }
//     // Tujuan ke direktori lain atau dengan nama file baru
//     else {
//         // Cari node tujuan yang kosong
//         for (i = 0; i < FS_MAX_NODE; i++) {
//             if (node_fs_buf.nodes[i].parent_index == 0x00 && node_fs_buf.nodes[i].data_index == 0x00) {
//                 dst_index = i;
//                 break;
//             }
//         }

//         if (dst_index == -1) {
//             printString("No free node available to move\n");
//             return;
//         }

//         // Pindahkan node dari sumber ke tujuan
//         node_fs_buf.nodes[dst_index].parent_index = cwd;
//         node_fs_buf.nodes[dst_index].data_index = node_fs_buf.nodes[src_index].data_index;
//         strcpy(node_fs_buf.nodes[dst_index].node_name, dst);

//         // Hapus node sumber
//         node_fs_buf.nodes[src_index].parent_index = 0x00;
//         node_fs_buf.nodes[src_index].data_index = 0x00;
//         node_fs_buf.nodes[src_index].node_name[0] = '\0';
//     }

//     // Tulis kembali node buffer ke disk
//     writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
//     writeSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

//     printString("\n");
// }


// TODO: 9. Implement cp function (SUDAH)
void cp(byte cwd, char *src, char *dst)
{
  struct node_fs node_fs_buf;
  int src_index = -1, dst_index = -1;
  int i;

  if (dst[0] == '\0')
  {
    // Tidak melakukan apapun jika filename kosong atau cetak file input nya ngga ada
    printString("Invalid file to copy\n");
    return; 
  }
  // baca node di readsector
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // nyari sumber node dari mana
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, src) == 0)
    {
      src_index = i;
      break;
    }
  }

  if (src_index == -1)
  {
    printString("File to copy not found\n");
    return;
  }
  // jika sumber nya folder (buat validasi biar hanya folder yang bisa di copy)
  if (node_fs_buf.nodes[src_index].data_index != FS_NODE_D_DIR)
  {
    printString("Cannot copy directories\n");
    return;
  }

  // nyari node
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == 0x00 &&
        node_fs_buf.nodes[i].data_index == 0x00)
    {
      dst_index = i;
      break;
    }
  }

  if (dst_index == -1)
  {
    // printString("No free node available to copy\n");
    return;
  }

  // copy sumber di tempatnya
  node_fs_buf.nodes[dst_index].parent_index = cwd;
  node_fs_buf.nodes[dst_index].data_index = node_fs_buf.nodes[src_index].data_index;
  strcpy(node_fs_buf.nodes[dst_index].node_name, dst);

  // nulis update filsystem node nya
  writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  printString("File copied successfully\n");
  // printString("\n");
}


// TODO: 10. Implement cat function (SUDAH SEMPURNA)
void cat(byte cwd, char *filename)
{
  struct file_metadata metadata;
  enum fs_return status;

  // Memeriksa apakah nama file diberikan dan tidak kosong
  if (filename[0] == '\0')
  {
    printString("Invalid file input\n");
    return; // Tidak melakukan apapun jika filename kosong atau cetak file input nya ngga ada
  }

  // Membaca file
  metadata.parent_index = cwd;
  strcpy(metadata.node_name, filename);
  fsRead(&metadata, &status);

  if (status == FS_SUCCESS)
  {
    printString(metadata.buffer);
    printString("\n");
  }

  else
  {
    printString("File not found\n");
  }
}

// TODO: 11. Implement mkdir function (SUDAH)
void mkdir(byte cwd, char *dirname)
{
  struct node_fs node_fs_buf;
  struct file_metadata metadata;
  enum fs_return status;
  int free_node_index = -1;
  int i;

  // (TAMBAHAN jika ngetik mkdir aja)
  if (dirname[0] == '\0')
  {
    // Tidak melakukan apapun jika folder name kosong atau cetak file input nya ngga ada
    printString("Invalid dir input\n");
    return;
  }

  // Membaca node sector dari filesystem
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // Mencari node yang kosong
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == 0x00 && node_fs_buf.nodes[i].data_index == 0x00)
    {
      free_node_index = i;
      break;
    }
  }

  if (free_node_index == -1)
  {
    // printString("Node not found\n");
    return;
  }

  // Mengatur node untuk direktori baru
  node_fs_buf.nodes[free_node_index].parent_index = cwd;
  node_fs_buf.nodes[free_node_index].data_index = FS_NODE_D_DIR;
  strcpy(node_fs_buf.nodes[free_node_index].node_name, dirname);

  // Menulis kembali node buffer ke disk
  writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector(((byte *)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // printString("Direktori berhasil dibuat\n");
}
