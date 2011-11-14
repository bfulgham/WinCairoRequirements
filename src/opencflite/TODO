--- Open CF-lite to-do list:
* CFRunLoop tests and implementation for Windows and Linux.
* Better examples
* List of projects using Open CF-lite.

--- Original CF-lite to-do list:
[Note: when it says "Apple has code" below, that usually
means "Apple has some code it could provide to start an
effort here", not "Apple has some code in the pipe, don't
bother with this item". Anyone known to be doing work on
any of these items will be listed here, including Apple.]
* Some classes have a fair number of assertions, nearly all
  related to parameter checking. More assertions are needed
  nearly everywhere. The assertions that are there have been
  often found to be valuable -- you just get a message about
  some bad parameter and there's the bug.
* More header doc is needed. CFArray.h and CFDictionary.h
  are models.
* An exception model, similar to Cocoa Foundation's. Apple
  has some code for this already, and try/catch model like
  C++ is simple enough to support; finally blocks a la Java
  don't seem to be practical within the confines of ANSI C.
* A CFFileDescriptor is needed which can act as a run loop
  source. Or maybe it should be CFPipeDescriptor. This is
  NOT something for general file handling -- just monitoring
  a file descriptor which is a pipe (for which there are
  notifications or other async activity).

