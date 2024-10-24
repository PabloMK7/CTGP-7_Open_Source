/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VoiceChatHandler.hpp
Open source lines: 97/97 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "Math.hpp"
#include "array"

namespace CTRPluginFramework {
    class VoiceChatHandler {
    private:
        static constexpr u32 SERVER_PORT = 2727;
        static constexpr u8 PACKET_VERSION = 1;

        struct PacketType {
            enum : u16 {
                JOIN_ROOM = 0,
                LEAVE_ROOM = 1,
                SET_PLAYERS = 2,
                UPDATE_MODE = 3,
                POSITION_INFO = 4,
                PING = 5,
            };
        };
        struct PacketHeader {
            u8 packetVersion;
            u8 packetType;
            u16 size;
        };
        struct JoinRoomPacket {
            PacketHeader header;
            char myName[64];
            char roomName[64];
        };
        struct LeaveRoomPacket {
            PacketHeader header;
        };
        struct SetPlayersPacket {
            PacketHeader header;
            char names[8][64];
        };
        struct UpdateModePacket {
            enum Mode : u8 {
                LOBBY = 0,
                RACE = 1,
            };
            PacketHeader header;
            Mode mode;
            bool isMirror;
        };
        struct PositionInfoPacket {
            PacketHeader header;
            Vector3T<s8> myFwd;
            Vector3T<s8> myUp;
            std::array<Vector3T<s16>, 8> positions;
            std::array<u8, 8> flags;
        };
        struct PingPacket {
            PacketHeader header;
        };

        static ThreadEx* voiceChatHandlerThread;
        static LightEvent threadEvent;
        static bool runThread;

        static Mutex packetAccessMutex;
        static u8 packetBuffer[1024];

        static void InitSocket();
        static void CloseSocket();
        
        static void HandlerThread(void* arg);
    public:
        static bool Initialized;
        static Handle* gameSocHandle;

        static bool Connected;
        static bool Error;

        static void JoinRoom(u32 region, u32 gatherID);
        static void LeaveRoom();
        static void UpdatePlayerNames();
        static void SetRaceMode(bool isRace, bool isMirror);
        static void SetPositions(const Vector3T<s8>& myFwd, const Vector3T<s8>& myUp, std::array<Vector3T<s16>, 8>& pos, std::array<u8, 8>& flags);

        static bool SendPacket(void* packet, size_t size, bool discardIfBusy);
        static void DisplayConnectMenu();

        static void OnSocketInit();
        static void OnSocketExit();
    };
}