/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartTimer.hpp
Open source lines: 151/151 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "math.h"

namespace CTRPluginFramework {
	class MarioKartTimer {
	public:

		static const u8 FramesPerSecond = 60;

        static constexpr u32 ToFrames(u32 minutes, u32 seconds, u32 milliseconds) {
            return ToFrames(minutes, seconds) + (milliseconds / 1000.f) * FramesPerSecond;
        }
        static constexpr u32 ToFrames(u32 minutes, u32 seconds) {
            return minutes * 60 * FramesPerSecond + seconds * FramesPerSecond;
        }

        inline constexpr MarioKartTimer() : Frames{0} {}

        inline constexpr MarioKartTimer(u32 frames) : Frames{frames} {}

        inline constexpr MarioKartTimer(u32 minutes, u32 seconds, u32 milliseconds) : MarioKartTimer(MarioKartTimer::ToFrames(minutes, seconds, milliseconds)) {}

        inline MarioKartTimer(const MarioKartTimer& other) : Frames(other.Frames) {}
        inline MarioKartTimer(MarioKartTimer&& other) noexcept : Frames(std::move(other.Frames)) {}

		inline void SetFrames(u32 frames) {
			Frames = frames;
		}

		inline u32 GetFrames() const {
			return Frames;
		}

		inline u32 GetMinutes() {
			return (Frames / FramesPerSecond) / 60;
		}

		inline u32 GetSeconds() {
			return (Frames / FramesPerSecond) % 60;
		}

		inline u32 GetMilliseconds() {
			float tot = ((float)Frames / FramesPerSecond);
			float floor = floorf(tot);
			return tot * 1000 - floor * 1000;
		}	

        std::string Format();

        inline MarioKartTimer operator+(const MarioKartTimer& right) const
        {
            return MarioKartTimer(this->Frames + right.Frames);
        }

        inline MarioKartTimer operator-(const MarioKartTimer& right) const
        {
            return MarioKartTimer(this->Frames - right.Frames);
        }

        inline MarioKartTimer operator*(const u32 amount) const
        {
            return MarioKartTimer(this->Frames * amount);
        }

        inline MarioKartTimer operator/(const u32 amount) const
        {
            return MarioKartTimer(this->Frames / amount);
        }

        inline bool operator>(const MarioKartTimer& right) const
        {
            return this->Frames > right.Frames;
        }

        inline bool operator>=(const MarioKartTimer& right) const
        {
            return this->Frames >= right.Frames;
        }

        inline bool operator<(const MarioKartTimer& right) const
        {
            return this->Frames < right.Frames;
        }

        inline bool operator<=(const MarioKartTimer& right) const
        {
            return this->Frames <= right.Frames;
        }

        inline bool operator==(const MarioKartTimer& right) const
        {
            return this->Frames == right.Frames;
        }

        inline bool operator!=(const MarioKartTimer& right) const
        {
            return this->Frames != right.Frames;
        }

        inline MarioKartTimer& operator++()
        {
            ++this->Frames;
            return *this;
        }

        inline MarioKartTimer operator++(int)
        {
            MarioKartTimer result(*this);
            ++(*this);
            return result;
        }

        inline MarioKartTimer& operator--()
        {
            --this->Frames;
            return *this;
        }

        inline MarioKartTimer operator--(int)
        {
            MarioKartTimer result(*this);
            --(*this);
            return result;
        }

        inline MarioKartTimer& operator=(const MarioKartTimer& other)
        {
            this->Frames = other.Frames;
            return *this;
        }

        inline MarioKartTimer& operator=(MarioKartTimer&& other) noexcept
        {
            this->Frames = other.Frames;
            return *this;
        }

	private:
		u32 Frames;
	};
}