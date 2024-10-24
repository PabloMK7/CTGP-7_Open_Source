/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VoiceChatHandler.cpp
Open source lines: 219/229 (95.63%)
*****************************************************/

#include "VoiceChatHandler.hpp"
#include "socCustom.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "Net.hpp"
#include "Unicode.h"
#include "SaveHandler.hpp"
#include "TCPStream.hpp"

namespace CTRPluginFramework {
    bool VoiceChatHandler::Initialized = false;
    Handle* VoiceChatHandler::gameSocHandle = nullptr; // 0x6689C8
    bool VoiceChatHandler::Connected = false;
    bool VoiceChatHandler::Error = false;
    ThreadEx* VoiceChatHandler::voiceChatHandlerThread = nullptr;
    LightEvent VoiceChatHandler::threadEvent;
    bool VoiceChatHandler::runThread = false;
    Mutex VoiceChatHandler::packetAccessMutex;
    u8 VoiceChatHandler::packetBuffer[1024] = {0};

    void VoiceChatHandler::InitSocket() {
        if (Initialized) return;
        Initialized = true;

        socCustomInit(gameSocHandle);

        Connected = false;
        Error = false;
        LightEvent_Init(&threadEvent, RESET_ONESHOT);
        runThread = true;
		#if CITRA_MODE == 0
        voiceChatHandlerThread = new ThreadEx(HandlerThread, 0x400, 0x20, System::IsNew3DS() ? 2 : 1);
        #else
        voiceChatHandlerThread = new ThreadEx(HandlerThread, 0x400, 0x20, 1);
        #endif
        voiceChatHandlerThread->Start(nullptr);
    }

    void VoiceChatHandler::CloseSocket() {
        if (!Initialized) return;
        Initialized = false;

        runThread = false;
        LightEvent_Signal(&threadEvent);
        voiceChatHandlerThread->Join(false);
        delete voiceChatHandlerThread;
        voiceChatHandlerThread = nullptr;

        socCustomExit();
    }

    void VoiceChatHandler::JoinRoom(u32 region, u32 gatherID) {

        u64 roomID = ((u64)region) << 32 | gatherID;

        JoinRoomPacket packet = { 0 };
        packet.header.packetType = PacketType::JOIN_ROOM;
        strncpy(packet.roomName, Utils::Format("r_%016llX", roomID).c_str(), 63);
        strncpy(packet.myName, Net::myServerName.c_str(), 63);

        SendPacket(&packet, sizeof(packet), false);
    }

    void VoiceChatHandler::LeaveRoom() {
        LeaveRoomPacket packet = { 0 };
        packet.header.packetType = PacketType::LEAVE_ROOM;

        SendPacket(&packet, sizeof(packet), false);
    }

    void VoiceChatHandler::UpdatePlayerNames() {
        SetPlayersPacket packet = { 0 };
        packet.header.packetType = PacketType::SET_PLAYERS;

        for (int i = 0; i < 8; i++) {
            strncpy(packet.names[i], Net::othersServerNames[i].c_str(), 63);
        }
        SendPacket(&packet, sizeof(packet), false);
    }

    void VoiceChatHandler::SetRaceMode(bool isRace, bool isMirror) {
        UpdateModePacket packet = { 0 };
        packet.header.packetType = PacketType::UPDATE_MODE;

        packet.mode = isRace ? UpdateModePacket::RACE : UpdateModePacket::LOBBY;
        packet.isMirror = isMirror;

        SendPacket(&packet, sizeof(packet), false);
    }

    void VoiceChatHandler::SetPositions(const Vector3T<s8>& myFwd, const Vector3T<s8>& myUp, std::array<Vector3T<s16>, 8>& pos, std::array<u8, 8>& flags) {
        PositionInfoPacket packet = { 0 };
        packet.header.packetType = PacketType::POSITION_INFO;

        packet.myFwd = myFwd;
        packet.myUp = myUp;
        packet.positions = pos;
        packet.flags = flags;

        SendPacket(&packet, sizeof(packet), true);
    }

    bool VoiceChatHandler::SendPacket(void* packet, size_t size, bool discardIfBusy) {
        if (Error || !runThread)
            return false;

        PacketHeader* hdr = reinterpret_cast<PacketHeader*>(packet);
        hdr->size = size;
        hdr->packetVersion = PACKET_VERSION;

        if (discardIfBusy) {
            PacketHeader* pending = (PacketHeader*)&packetBuffer;
            if (pending->packetVersion != 0)
                return false;
        }

        {
            Lock lock(packetAccessMutex);
            memcpy(packetBuffer, packet, size);
        }
        LightEvent_Signal(&threadEvent);
        return true;
    }

    void VoiceChatHandler::HandlerThread(void* args)
    {
        TCPStream stream(SERVER_PORT);
        if (stream.GetLastErrno() < 0) {
            Error = true;
            return;
        }

        stream.SetRunCondition(&runThread);

        bool con = stream.WaitConnection();
        if (!runThread)
            return;

        if (!con) {
            Error = true;
            return;
        }

        Connected = true;

        PacketHeader* hdr = (PacketHeader*)&packetBuffer;

        while (runThread) {
            bool timedOut = LightEvent_WaitTimeout(&threadEvent, 5000000000ULL) != 0;
            if (timedOut && runThread && !Error) {
                PingPacket packet = { 0 };
                packet.header.packetType = PacketType::PING;
                packet.header.size = sizeof(packet);
                packet.header.packetVersion = PACKET_VERSION;
                Error = !stream.Write(&packet, sizeof(packet));
                continue;
            }

            Lock lock(packetAccessMutex);
            if (!runThread || hdr->packetVersion == 0)
                continue;

            if (!Error) Error = !stream.Write(hdr, hdr->size);

            hdr->packetVersion = 0;
        }
    }

    void VoiceChatHandler::DisplayConnectMenu() {
        (*PluginMenu::GetRunningInstance()) -= DisplayConnectMenu;

        Process::Pause();

        InitSocket();

        Keyboard kbd(CenterAlign(NAME("proximity_chat")) + HorizontalSeparator() + "\n" + Utils::Format(NAME("proximity_chat_desc").c_str(), (ToggleDrawMode(Render::FontDrawMode::BOLD) + TCPStream::GetHostIP() + ToggleDrawMode(Render::FontDrawMode::BOLD)).c_str()));
        kbd.CanAbort(true);
        kbd.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
            if (event.type == KeyboardEvent::EventType::FrameTop) {
                if (Connected) {
                    k.Close();
                }
                if (Error) {
                    k.GetMessage() = NAME("proximity_chat") + HorizontalSeparator() + "\n" + NOTE("proximity_chat_desc");
                }
            }
        });

        kbd.Populate({""});
        int res = kbd.Open();

        if (!Connected) {
            CloseSocket();
        }

        Process::Play();
    }

    void VoiceChatHandler::OnSocketInit() {
        if (!SaveHandler::saveData.flags1.enableVoiceChat)
            return;
        
        (*PluginMenu::GetRunningInstance()) += DisplayConnectMenu;
    }

    void VoiceChatHandler::OnSocketExit() {
        CloseSocket();
    }
}