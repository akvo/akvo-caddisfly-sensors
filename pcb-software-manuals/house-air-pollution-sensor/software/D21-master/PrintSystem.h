#ifndef PRINTSYSTEM_H_
#define PRINTSYSTEM_H_

#include <Arduino.h>
#include "MacroOverload.h"

/*
 * Helper class to help decouple console and debug streams. 
 * Provides some nice macros also to avoid string concatenation.
 */

#define DEBUG_STR "[ DEBUG ]"
#define ERROR_STR "[ ERROR ]"

#define DEBUG

// Debug
#ifdef DEBUG
#define debugPrint(...) if (printSystem.getDebug()) { unsafeDebugPrint(__VA_ARGS__); }
#define debugPrintLn(...) VFUNC(debugPrintLn, __VA_ARGS__)

#define unsafeDebugPrint(...) printSystem.getDebug()->print(__VA_ARGS__)
#define unsafeDebugPrintLn(...) printSystem.getDebug()->println(__VA_ARGS__)

#define debugPrintLn1(last) if (printSystem.getDebug()) { unsafeDebugPrintLn(last); }
#define debugPrintLn2(_1,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrintLn(last); }
#define debugPrintLn3(_1,_2,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrintLn(last); }
#define debugPrintLn4(_1,_2,_3,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrintLn(last); }
#define debugPrintLn5(_1,_2,_3,_4,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrintLn(last); }
#define debugPrintLn6(_1,_2,_3,_4,_5,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrint(_5); unsafeDebugPrintLn(last); }
#define debugPrintLn7(_1,_2,_3,_4,_5,_6,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrint(_5); unsafeDebugPrint(_6); unsafeDebugPrintLn(last); }
#define debugPrintLn8(_1,_2,_3,_4,_5,_6,_7,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrint(_5); unsafeDebugPrint(_6); unsafeDebugPrint(_7); unsafeDebugPrintLn(last); }
#define debugPrintLn9(_1,_2,_3,_4,_5,_6,_7,_8,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrint(_5); unsafeDebugPrint(_6); unsafeDebugPrint(_7); unsafeDebugPrint(_8); unsafeDebugPrintLn(last); }
#define debugPrintLn10(_1,_2,_3,_4,_5,_6,_7,_8,_9,last) if (printSystem.getDebug()) { unsafeDebugPrint(_1); unsafeDebugPrint(_2); unsafeDebugPrint(_3); unsafeDebugPrint(_4); unsafeDebugPrint(_5); unsafeDebugPrint(_6); unsafeDebugPrint(_7); unsafeDebugPrint(_8); unsafeDebugPrint(_9); unsafeDebugPrintLn(last); }

#else
#define debugPrint(last)
#define debugPrintLn(...)
#endif

// Console
#define consolePrint(last) if (printSystem.getConsole()) { unsafeConsolePrint(last); }
#define consolePrintLn(...) VFUNC(consolePrintLn, __VA_ARGS__)

#define unsafeConsolePrint(last) printSystem.getConsole()->print(last)
#define unsafeConsolePrintLn(last) printSystem.getConsole()->println(last)

#define consolePrintLn1(last) if (printSystem.getConsole()) { unsafeConsolePrintLn(last); }
#define consolePrintLn2(_1,last) if (printSystem.getConsole()) { unsafeConsolePrint(_1); unsafeConsolePrintLn(last); }
#define consolePrintLn3(_1,_2,last) if (printSystem.getConsole()) { unsafeConsolePrint(_1); unsafeConsolePrint(_2); unsafeConsolePrintLn(last); }
#define consolePrintLn4(_1,_2,_3,last) if (printSystem.getConsole()) { unsafeConsolePrint(_1); unsafeConsolePrint(_2); unsafeConsolePrint(_3); unsafeConsolePrintLn(last); }
#define consolePrintLn5(_1,_2,_3,_4,last) if (printSystem.getConsole()) { unsafeConsolePrint(_1); unsafeConsolePrint(_2); unsafeConsolePrint(_3); unsafeConsolePrint(_4); unsafeConsolePrintLn(last); }

class PrintSystem
{
public:
    PrintSystem() : consoleStream(0), debugStream(0) { }

    void setConsole(Stream& stream) { consoleStream = &stream; }
    void setDebug(Stream& stream) { debugStream = &stream; }

    Stream* getConsole() { return consoleStream; }
    Stream* getDebug() { return debugStream; }
protected:
    Stream* consoleStream;
    Stream* debugStream;
};

extern PrintSystem printSystem;

#endif /* PRINTSYSTEM_H_ */
