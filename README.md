# Torrent-RD-PC

Basically what the description says. Open some file via command line argument, then it will download it somewhere on your PC.

# How to use the executable
Download Torrent-RD-PC.exe from the release page and follow step 2 and onwards in the [Build Instructions](#Build-Instructions) section.

# Build Instructions
1. Compile it somewhere to your PC, just make sure it can read/write to that directory.
If using g++, make sure to use the -municode flag or else it will not compile.
2. Create settings.txt and replace "string" with the real values you want. The outdir should not be surrounded with quotes. Do `C:\foobar`, not `"C:\foobar"`.

  Example `settings.txt`:
  ```
  token=string
  outdir=string
   ```
3. Make sure to save settings.txt to the same directory as the executable.
4. Make sure to have a copy of cURL accessible from your PATH.
If not, please install cURL.cURL from WinGet (recommended) or go to https://curl.se/download.html.
5. Run the executable with the .torrent file directory as a command line argument, and you're golden. 
You can also drag and drop files on the executable!
This program should work with any software that can properly pass file directories as the only argument.

# This software is free to use, modify, and hopefully improve.
# The author is not responsible for any misuse of said software.
# This tool should only be used to download torrents that the user has the legal right to download.
# No help whatsoever will be provided regarding potentially unauthorized torrents not functioning with Torrent-RD-PC.