/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PROCESSOR_NAME
    #define PROCESSOR_NAME "Unknown"
#endif

#undef LOGA
#define LOGA(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][action] ", __VA_ARGS__)

#undef LOGB
#define LOGB(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][buffer] ", __VA_ARGS__)

#undef LOGC
#define LOGC(...) \
    getOELogger().LOGConsole("[" PROCESSOR_NAME "] ", __VA_ARGS__)

#undef LOGD
#ifdef DEBUG
#define LOGD(...) \
    getOELogger().LOGConsole("[" PROCESSOR_NAME "][debug] ", __VA_ARGS__)
#else
#define LOGD(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][debug] ", __VA_ARGS__)
#endif

#undef LOGDD
#define LOGDD(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][ddebug] ", __VA_ARGS__)

#undef LOGE
#define LOGE(...) \
    getOELogger().LOGError("[" PROCESSOR_NAME "] ***ERROR*** ", __VA_ARGS__)

// Not applicable for plugins
#undef LOGF
#undef LOGG