#!/usr/bin/python3

import re

cpragma = re.compile('mark *- *([A-Za-z0-9_]+) *-?\n')
cclass = re.compile('^class +([A-Za-z0-9_]+) *;')

def substname(name):
    return name.replace('Color', 'Colour')\
               .replace('TimeScale', 'Timescale')\
               .replace('LfpSupersampled', 'Supersampled')

## SPLIT UP ORIGINAL HEADER FILE
with open('LfpDisplayCanvas.h.orig', 'r') as fh:
    txt = fh.read()

classes = { 'LfpDisplayNode': 1 }
sections = txt.split('#pragma')

# Parse header
hdr = []
headsec = sections.pop(0)
for line in headsec.split('\n'):
    m = cclass.search(line)
    if m:
        pass
    else:
        hdr.append(line)

# Find list of classes
for sec in sections:
    m = cpragma.search(sec)
    if m:
        name = substname(m.group(1))
        classes[name] = 1

# Write general include file
with open(f'LfpDisplayClasses.h', 'w') as fh:
    fh.write('''// LfpDisplayClasses.h - part of LfpDisplayNode
#ifndef LFPDISPLAYCLASSES_H
#define LFPDISPLAYCLASSES_H

namespace LfpViewer {
    constexpr int MAX_N_CHAN = 2048;
    constexpr int MAX_N_SAMP = 5000;
    constexpr int MAX_N_SAMP_PER_PIXEL = 100;
    constexpr int CHANNEL_TYPES = 3;

''')
    for cls in classes:
        fh.write(f'    class {cls};\n')
    fh.write('''
};
#endif
''')


# Parse each section
included = {}
for sec in sections:
    secinclude = {}
    m = cpragma.search(sec)
    if m:
        name = substname(m.group(1))
    else:
        raise Exception(f'Did not understand section: {sec[:80]}')
    with open(f'{name}.h', 'w') as fh:
        # Write (modified) header
        empty = False
        for line in hdr:
            if '#ifndef' in line:
                fh.write(f'#ifndef __{name.upper()}_H__\n')
            elif '#define __LFPDISPLAYCANVAS' in line:
                fh.write(f'#define __{name.upper()}_H__\n')
            elif '#include "LfpDisplayNode.h"' in line:
                pass
            elif 'namespace' in line:
                # Write include files here
                fh.write('#include "LfpDisplayClasses.h"\n')
                fh.write('#include "LfpDisplayNode.h"\n')
                for cls in classes:
                    if cls!=name and re.compile(f'\\b{cls}\\b').search(sec):
                        secinclude[cls] = 1
                        if re.compile(f'public +{cls}\\b').search(sec):
                            fh.write(f'#include "{cls}.h"\n')
                fh.write(line + '\n')
            else:
                if re.compile('^ *$').match(line):
                    # Compress multiple empty lines to one
                    if not empty:
                        fh.write(line + '\n')
                    empty = True
                else:
                    empty = False    
                    fh.write(line + '\n')
        included[name] = secinclude
        
        # Write body
        empty = False
        for line in sec.split('\n'):
            if cpragma.search(line + '\n'):
                fh.write('#pragma ' + line + '\n')
            else:
                if re.compile('^ *$').match(line):
                    # Compress multiple empty lines to one
                    if not empty:
                        fh.write(line + '\n')
                    empty = True
                else:
                    empty = False    
                    fh.write(line + '\n')

        # Write end except for final section which has its own end
        if '#endif  // __LFPDISPLAYCAN' in sec:
            pass
        else:
            fh.write('}; // namespace\n')
            fh.write('#endif\n')

# Fix up some missing includes
for k in range(1):
    for cls in list(included.keys()):
        for inc in list(included[cls].keys()):
            if inc in included:
                for sub in included[inc]:
                    included[cls][sub] = 1

## SPLIT UP ORIGINAL CPP FILE
with open('LfpDisplayCanvas.cpp.orig', 'r') as fh:
    txt = fh.read()

sections = txt.split('#pragma')

# Parse header
hdr = []
headsec = sections.pop(0)
for line in headsec.split('\n'):
    m = cclass.search(line)
    if m:
        classes[m.group(1)] = 1;
    else:
        hdr.append(line)

for sec in sections:
    m = cpragma.search(sec)
    if m:
        name = substname(m.group(1))
    else:
        raise Exception(f'Did not understand section: {sec[:80]}')
    with open(f'{name}.cpp', 'w') as fh:
        # Write (modified) header
        empty = False
        for line in hdr:
            if '#include "LfpDisplayCanvas.h"' in line:
                # Write include files here
                fh.write(f'#include "{name}.h"\n')
                for cls in classes:
                    if cls!=name and (cls in included[name] 
                                      or re.compile(f'\\b{cls}\\b').search(sec)):
                        fh.write(f'#include "{cls}.h"\n')
                fh.write('\n')
            else:
                if re.compile('^ *$').match(line):
                    # Compress multiple empty lines to one
                    if not empty:
                        fh.write(line + '\n')
                    empty = True
                else:
                    empty = False    
                    fh.write(line + '\n')
        
        # Write body
        empty = False
        for line in sec.split('\n'):
            if cpragma.search(line + '\n'):
                fh.write('#pragma ' + line + '\n')
            else:
                if re.compile('^ *$').match(line):
                    # Compress multiple empty lines to one
                    if not empty:
                        fh.write(line + '\n')
                    empty = True
                else:
                    empty = False    
                    fh.write(line + '\n')
        
