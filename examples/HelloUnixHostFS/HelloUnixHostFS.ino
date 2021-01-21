#include <stdio.h> // remove()
#include <ftw.h> // nftw()
#include <Arduino.h>

#if defined(UNIX_HOST_DUINO)
  #include <UnixHostFS.h>
#elif defined(ESP8266)
  #include <LittleFS.h>
#else
  #error Unsupported platform
#endif

using fs::UnixHostFS;

int removeFile(
    const char *fpath,
    const struct stat *sb,
    int typeflag,
    struct FTW *ftwbuf)
{
  if (typeflag == FTW_F) {
    printf("File: %s\n", fpath);
  } else if (typeflag == FTW_SL) {
    printf("Symlink: %s\n", fpath);
  } if (typeflag == FTW_DP) {
    printf("Post Dir: %s\n", fpath);
  }
  int status = 0;
  if (ftwbuf->level != 0) {
    status = remove(fpath);
  }
  return status;
}

void removeFtw() {
  SERIAL_PORT_MONITOR.println("== FTW List '/'");
  nftw("fsdata", removeFile, 5, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
}

void listDir(FS& fileSystem) {
  SERIAL_PORT_MONITOR.println("== Dir List '/'");

  // Open dir folder
  Dir dir = fileSystem.openDir("/");
  // Cycle all the content
  while (dir.next()) {
    // Print directory entry
    SERIAL_PORT_MONITOR.print("Dir: ");
    SERIAL_PORT_MONITOR.print(dir.fileName());
    if (dir.isDirectory()) {
      SERIAL_PORT_MONITOR.println('/');
    } else {
      SERIAL_PORT_MONITOR.println();
    }

    // If element have a size display It else write 0
    SERIAL_PORT_MONITOR.print("File: ");
    if (dir.fileSize()) {
        File f = dir.openFile("r");
        SERIAL_PORT_MONITOR.print(f.name());
        SERIAL_PORT_MONITOR.print(':');
        SERIAL_PORT_MONITOR.println(f.size());
        f.close();
    } else {
        SERIAL_PORT_MONITOR.println("0");
    }
  }
}

void writeFile(FS& fileSystem) {
  SERIAL_PORT_MONITOR.println("== Writing 'testfile.txt'");

  File f = fileSystem.open("testfile.txt", "w");
  f.println("This is a test");
  f.println(42);
  f.println(42.0);
  f.println(42, 16);
  f.close();

  bool exists = fileSystem.exists("testfile.txt");
  if (exists) {
    SERIAL_PORT_MONITOR.println("'testfile.txt' created");
  } else {
    SERIAL_PORT_MONITOR.println("ERROR: 'testfile.txt' not created");
  }
}

void readFile(FS& fileSystem) {
  SERIAL_PORT_MONITOR.println("== Reading 'testfile.txt'");

  File f = fileSystem.open("testfile.txt", "r");
  while (f.available()) {
    String s = f.readStringUntil('\r');
    SERIAL_PORT_MONITOR.print(s);
    f.read();
    SERIAL_PORT_MONITOR.println();
  }
  f.close();
}

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // some boards reboot twice
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // For Leonardo/Micro

#if defined(UNIX_HOST_DUINO)
  SERIAL_PORT_MONITOR.print(F("Inizializing UnixHostFS..."));
  FS& fileSystem = UnixHostFS;
#else
  SERIAL_PORT_MONITOR.print(F("Inizializing LittleFS..."));
  FS& fileSystem = LittleFS;
#endif

  if (fileSystem.begin()){
    SERIAL_PORT_MONITOR.println(F("done."));
    listDir(fileSystem);
    writeFile(fileSystem);
    readFile(fileSystem);
    removeFtw();
  } else {
    SERIAL_PORT_MONITOR.println(F("fail."));
  }

#if defined(UNIX_HOST_DUINO)
  exit(0);
#endif
}

void loop() {
}
