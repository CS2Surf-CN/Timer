#include "surf_replay.h"
#include <utils/utils.h>
#include <thread>
#include <fstream>
#include <fmt/format.h>
#include <zlib/zlib.h>

struct replay_packed_button_t {
	u8 attack: 1 {};
	u8 jump: 1 {};
	u8 duck: 1 {};
	u8 use: 1 {};
	u8 attack2: 1 {};
	u8 reload: 1 {};
	u8 speed: 1 {};
	u8 inspect: 1 {};

	replay_packed_button_t() = default;

	replay_packed_button_t(u64 button) {
		Pack(button);
	}

	// clang-format off

	void Pack(u64 button) {
		if (button & IN_ATTACK) attack = true;
		if (button & IN_JUMP) jump = true;
		if (button & IN_DUCK) duck = true;
		if (button & IN_USE) use = true;
		if (button & IN_ATTACK2) attack2 = true;
		if (button & IN_RELOAD) reload = true;
		if (button & IN_SPEED) speed = true;
		if (button & IN_LOOK_AT_WEAPON) inspect = true;
	}

	u64 Unpack() const {
		u64 button = {};
		if (attack) button |= IN_ATTACK;
		if (jump) button |= IN_JUMP;
		if (duck) button |= IN_DUCK;
		if (use) button |= IN_USE;
		if (attack2) button |= IN_ATTACK2;
		if (reload) button |= IN_RELOAD;
		if (speed) button |= IN_SPEED;
		if (inspect) button |= IN_LOOK_AT_WEAPON;

		return button;
	}

	// clang-format on
};

constexpr auto REPLAY_ARRAY_HEAD = "===ARRAY_HEAD===";
constexpr auto REPLAY_ARRAY_TAIL = "===ARRAY_TAIL===";
constexpr size_t REPLAY_FRAME_SIZE = sizeof(Vector2D) + sizeof(replay_frame_data_t::pos) + sizeof(replay_frame_data_t::flags) + sizeof(replay_frame_data_t::mt) + sizeof(replay_packed_button_t);

static void ReadStreamString(std::ifstream& file, std::string& outStr) {
	uint32_t len = 0;
	file.read(reinterpret_cast<char*>(&len), sizeof(len));
	outStr.resize(len);
	file.read(&outStr[0], len);
}

static void WriteStreamString(std::ofstream& file, const std::string& str) {
	uint32_t length = static_cast<uint32_t>(str.size());
	file.write(reinterpret_cast<const char*>(&length), sizeof(length));
	file.write(str.data(), length);
}

void replay_file_header_t::ReadFromStream(std::ifstream& in) {
	ReadStreamString(in, format);
	in.read(reinterpret_cast<char*>(&version), sizeof(version));
	ReadStreamString(in, map);
	in.read(reinterpret_cast<char*>(&tickrate), sizeof(tickrate));
	in.read(reinterpret_cast<char*>(&info), sizeof(info));
}

void replay_file_header_t::WriteToStream(std::ofstream& out) const {
	WriteStreamString(out, format);
	out.write(reinterpret_cast<const char*>(&version), sizeof(version));
	WriteStreamString(out, map);
	out.write(reinterpret_cast<const char*>(&tickrate), sizeof(tickrate));
	out.write(reinterpret_cast<const char*>(&info), sizeof(info));
}

std::string CSurfReplayPlugin::BuildReplayPath(const i8 style, const TimerTrack_t track, const i8 stage, const std::string_view map) {
	std::string sDirPath = UTIL::PATH::Join(UTIL::GetWorkingDirectory(), "replay", map);
	try {
		if (!std::filesystem::exists(sDirPath)) {
			std::filesystem::create_directories(sDirPath);
		}
	} catch (const std::filesystem::filesystem_error& e) {
		LOG::Error("[BuildReplayPath] Failed to create directory: %s\n", e.what());
		return "";
	}

	auto getTrackFileName = [](const TimerTrack_t track) -> std::string {
		switch (track) {
			case EZoneTrack::Track_Main:
				return "Main";
			default:
				return fmt::format("B{}", (i8)track);
		}
	};

	std::string sMaybeStage = (stage == 0) ? "" : fmt::format("-s{}", stage);
	return fmt::format("{}/{}-{}{}.replay", sDirPath, SURF::GetStyleShortName(style), getTrackFileName(track), sMaybeStage);
}

void CSurfReplayPlugin::AsyncWriteReplayFile(const replay_run_info_t& info, const ReplayArray_t& vFrames) {
	std::string sMap = UTIL::GetGlobals()->mapname.ToCStr();
	std::string sFilePath = BuildReplayPath(info.style, info.track, info.stage, sMap);
	std::thread([info, vFrames, sFilePath, sMap]() {
		std::ofstream file(sFilePath, std::ios::out | std::ios::binary);
		if (!file.good()) {
			LOG::Error("Failed to WriteReplayFile: %s", sFilePath.data());
			SDK_ASSERT(false);
			return;
		}

		replay_file_header_t header;
		header.map = sMap;
		header.tickrate = SURF_TICKRATE;
		header.info = info;
		header.WriteToStream(file);

		WriteStreamString(file, REPLAY_ARRAY_HEAD);

		std::vector<char> frames_buffer;

		frames_buffer.resize(info.framelength * REPLAY_FRAME_SIZE);

		for (size_t i = 0, offset = 0; i < info.framelength; i++) {
			const auto& frame = vFrames.at(i);

			std::memcpy(frames_buffer.data() + offset, &frame.ang, sizeof(Vector2D));
			offset += sizeof(Vector2D);

			std::memcpy(frames_buffer.data() + offset, &frame.pos, sizeof(frame.pos));
			offset += sizeof(frame.pos);

			std::memcpy(frames_buffer.data() + offset, &frame.flags, sizeof(frame.flags));
			offset += sizeof(frame.flags);

			std::memcpy(frames_buffer.data() + offset, &frame.mt, sizeof(frame.mt));
			offset += sizeof(frame.mt);

			replay_packed_button_t packButton(frame.buttons);
			std::memcpy(frames_buffer.data() + offset, &packButton, sizeof(packButton));
			offset += sizeof(packButton);
		}

		const uLong sourceSize = static_cast<uLong>(frames_buffer.size());
		uLongf destSize = compressBound(sourceSize);
		std::vector<Bytef> compressedData(destSize);

		int ret = compress2(compressedData.data(), &destSize, reinterpret_cast<const Bytef*>(frames_buffer.data()), sourceSize, Z_BEST_COMPRESSION);

		if (ret != Z_OK) {
			LOG::Error("Failed to WriteReplayFile: %s, Reason: zlib compress failed", sFilePath.data());
			SDK_ASSERT(false);
			return;
		}

		file.write(reinterpret_cast<const char*>(&destSize), sizeof(destSize));
		file.write(reinterpret_cast<const char*>(compressedData.data()), destSize);

		WriteStreamString(file, REPLAY_ARRAY_TAIL);
	}).detach();
}

bool CSurfReplayPlugin::ReadReplayFile(const std::string_view path, ReplayArray_t& out) {
	std::ifstream file(path.data(), std::ios::in | std::ios::binary);
	if (!file.good()) {
		LOG::Error("Failed to read replay file: %s, Reason: file not found!", path.data());
		return false;
	}

	replay_file_header_t header;
	header.ReadFromStream(file);

	std::string sHead;
	ReadStreamString(file, sHead);

	if (sHead != std::string(REPLAY_ARRAY_HEAD)) {
		LOG::Error("Failed to read replay file: %s, Reason: array head not match!", path.data());
		return false;
	}

	uLongf compressed_size = 0;
	file.read(reinterpret_cast<char*>(&compressed_size), sizeof(compressed_size));

	std::vector<Bytef> compressed_data(compressed_size);
	file.read(reinterpret_cast<char*>(compressed_data.data()), compressed_size);

	auto iFrameLen = header.info.framelength;
	uLongf decompressed_size = static_cast<uLongf>(iFrameLen * REPLAY_FRAME_SIZE);
	std::vector<Bytef> decompressed_data(decompressed_size);
	int ret = uncompress(decompressed_data.data(), &decompressed_size, compressed_data.data(), compressed_size);
	if (ret != Z_OK) {
		LOG::Error("Failed to read replay file: %s, Reason: zlib uncompress error", path.data());
		return false;
	}

	out = {};
	out.reserve(iFrameLen);
	for (size_t i = 0, offset = 0; i < iFrameLen; i++) {
		replay_frame_data_t frame;

		frame.ang.z = 0;
		std::memcpy(&frame.ang, decompressed_data.data() + offset, sizeof(Vector2D));
		offset += sizeof(Vector2D);

		std::memcpy(&frame.pos, decompressed_data.data() + offset, sizeof(frame.pos));
		offset += sizeof(frame.pos);

		std::memcpy(&frame.flags, decompressed_data.data() + offset, sizeof(frame.flags));
		offset += sizeof(frame.flags);

		std::memcpy(&frame.mt, decompressed_data.data() + offset, sizeof(frame.mt));
		offset += sizeof(frame.mt);

		replay_packed_button_t packButton;
		std::memcpy(&packButton, decompressed_data.data() + offset, sizeof(packButton));
		offset += sizeof(packButton);

		frame.buttons = packButton.Unpack();

		out.emplace_back(frame);
	}

	std::string sTail;
	ReadStreamString(file, sTail);

	return true;
}
