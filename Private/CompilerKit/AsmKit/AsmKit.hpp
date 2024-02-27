/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/CompilerKit.hpp>
#include <CompilerKit/Defines.hpp>
#include <CompilerKit/StdKit/String.hpp>

namespace CompilerKit {
//
//	@brief Frontend to Assembly mountpoint.
//
class AssemblyMountpoint {
 public:
  explicit AssemblyMountpoint() = default;
  virtual ~AssemblyMountpoint() = default;

  MPCC_COPY_DEFAULT(AssemblyMountpoint);

  //@ brief compile to object file.
  // Example C++ -> MASM -> AE object.
  virtual Int32 CompileToFormat(StringView& src, Int32 arch) = 0;
};

/// @brief Simple assembly factory
class AssemblyFactory final {
 public:
  explicit AssemblyFactory() = default;
  ~AssemblyFactory() = default;

  MPCC_COPY_DEFAULT(AssemblyFactory);

 public:
  enum {
    kArchAMD64,
    kArch32x0,
    kArch64x0,
    kArchRISCV,
    kArchUnknown,
  };

  Int32 Compile(StringView& sourceFile, const Int32& arch) noexcept;

  void Mount(AssemblyMountpoint* mountPtr) noexcept;
  AssemblyMountpoint* Unmount() noexcept;

 private:
  AssemblyMountpoint* fMounted{nullptr};
};

union NumberCastBase {
  NumberCastBase() = default;
  ~NumberCastBase() = default;
};

union NumberCast64 final {
  NumberCast64() = default;
  explicit NumberCast64(UInt64 raw) : raw(raw) {}
  ~NumberCast64() { raw = 0; }

  CharType number[8];
  UInt64 raw;
};

union NumberCast32 final {
  NumberCast32() = default;
  explicit NumberCast32(UInt32 raw) : raw(raw) {}
  ~NumberCast32() { raw = 0; }

  CharType number[4];
  UInt32 raw;
};

union NumberCast16 final {
  NumberCast16() = default;
  explicit NumberCast16(UInt16 raw) : raw(raw) {}
  ~NumberCast16() { raw = 0; }

  CharType number[2];
  UInt16 raw;
};

union NumberCast8 final {
  NumberCast8() = default;
  explicit NumberCast8(UInt8 raw) : raw(raw) {}
  ~NumberCast8() { raw = 0; }

  CharType number;
  UInt8 raw;
};

class Encoder {
 public:
  explicit Encoder() = default;
  ~Encoder() = default;

  MPCC_COPY_DEFAULT(Encoder);

  virtual std::string CheckLine(std::string& line, const std::string& file) = 0;
  virtual bool WriteLine(std::string& line, const std::string& file) = 0;
  virtual bool WriteNumber(const std::size_t& pos, std::string& from_what) = 0;
};

#ifdef __ASM_NEED_AMD64__

class EncoderAMD64 final : public Encoder {
 public:
  explicit EncoderAMD64() = default;
  ~EncoderAMD64() = default;

  MPCC_COPY_DEFAULT(EncoderAMD64);

  virtual std::string CheckLine(std::string& line,
                                const std::string& file) override;
  virtual bool WriteLine(std::string& line, const std::string& file) override;
  virtual bool WriteNumber(const std::size_t& pos,
                           std::string& from_what) override;

  virtual bool WriteNumber16(const std::size_t& pos, std::string& from_what);
  virtual bool WriteNumber32(const std::size_t& pos, std::string& from_what);
  virtual bool WriteNumber8(const std::size_t& pos, std::string& from_what);
};

#endif  // __ASM_NEED_AMD64__

#ifdef __ASM_NEED_64x0__

class Encoder64x0 final : public Encoder {
 public:
  explicit Encoder64x0() = default;
  ~Encoder64x0() = default;

  MPCC_COPY_DEFAULT(Encoder64x0);

  virtual std::string CheckLine(std::string& line,
                                const std::string& file) override;
  virtual bool WriteLine(std::string& line, const std::string& file) override;
  virtual bool WriteNumber(const std::size_t& pos,
                           std::string& from_what) override;
};

#endif  // __ASM_NEED_64x0__

#ifdef __ASM_NEED_32x0__

class Encoder32x0 final : public Encoder {
 public:
  explicit Encoder32x0() = default;
  ~Encoder32x0() = default;

  MPCC_COPY_DEFAULT(Encoder32x0);

  virtual std::string CheckLine(std::string& line,
                                const std::string& file) override;
  virtual bool WriteLine(std::string& line, const std::string& file) override;
  virtual bool WriteNumber(const std::size_t& pos,
                           std::string& from_what) override;
};

#endif  // __ASM_NEED_32x0__
}  // namespace CompilerKit
