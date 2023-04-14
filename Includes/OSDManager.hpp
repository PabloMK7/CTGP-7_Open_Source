/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: OSDManager.hpp
Open source lines: 63/63 (100.00%)
*****************************************************/

#ifndef OSD_MANAGER_HPP
#define OSD_MANAGER_HPP

#include "3DS.h"
#include "CTRPluginFramework.hpp"

#include <map>
#include <tuple>

namespace CTRPluginFramework
{
    #define OSDManager (*_OSDManager::GetInstance())

    using OSDMITuple = std::tuple<bool, std::string, u32, u32, bool>;
    struct OSDMI
    {
        OSDMI &operator=(const std::string &str);
        OSDMI &operator=(const OSDMITuple &tuple);
        OSDMI &SetPos(u32 posX, u32 posY);
        OSDMI &SetScreen(bool topScreen);
        OSDMI &Enable(void);
        OSDMI &Disable(void);
    private:
        friend class  _OSDManager;
        explicit OSDMI(OSDMITuple &tuple);

        OSDMITuple  &data;
    };

    class _OSDManager
    {
    public:
        ~_OSDManager(void);

        static _OSDManager  *GetInstance(void);      
        
        OSDMI   operator[](const std::string &key);
        void    Remove(const std::string &key);
        void    Lock(void);
        void    Unlock(void);
    private:

        _OSDManager(void);

        static bool     OSDCallback(const Screen &screen);

        static _OSDManager *_singleton;

        LightLock   _lock;
        std::map<std::string, std::tuple<bool, std::string, u32, u32, bool>> _items;
    };
}

#endif
