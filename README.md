# VCapture

A simple screenshotting utility for windows, just hold CTRL and SHIFT then press 'S'.





TODO:
- Clean up, the code it's extremely messy
- Might have a memory leak with GlobalAlloc when saving to clipboard
- Replace VirtualAlloc in the Capture.h CopyImageToClipboard() with alloca
- Pressing ctrl shift s when cropping causes strange behavior
