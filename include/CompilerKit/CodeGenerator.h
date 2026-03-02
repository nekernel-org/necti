// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef NECTAR_COMPILERKIT_CODEGENERATOR_H
#define NECTAR_COMPILERKIT_CODEGENERATOR_H

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/Macros.h>

#include <CompilerKit/ErrorOr.h>
#include <CompilerKit/Ref.h>

#include <cstring>

#define CK_ASSEMBLY_INTERFACE : public ::CompilerKit::IAssembly
#define CK_ENCODER : public ::CompilerKit::IAssemblyEncoder

namespace CompilerKit {
class AssemblyFactory;
class IAssembly;

/// =========================================================== ///
/// @brief Simple assembly factory
/// =========================================================== ///
class AssemblyFactory final {
 public:
  explicit AssemblyFactory() = default;
  ~AssemblyFactory()         = default;

  NECTAR_COPY_DEFAULT(AssemblyFactory);

 public:
  enum {
    kArchInvalid = 0,
    kArchAMD64   = 100,
    kArch32x0,
    kArch64x0,
    kArchRISCV,
    kArchPowerPC,
    kArchAARCH64,
    kArchUnknown,
    kArchCount = kArchUnknown - kArchAMD64,
  };

  Int32              Compile(STLString sourceFile, const Int32& arch);
  void               Mount(WeakRef<IAssembly> mountPtr);
  WeakRef<IAssembly> Unmount() noexcept;

 private:
  IAssembly* fMounted{nullptr};
};

/// =========================================================== ///
///	@brief Assembly to binary generator class.
/// @note This interface creates according to the CPU target of the child class.
/// =========================================================== ///
class IAssembly {
 public:
  explicit IAssembly() = default;
  virtual ~IAssembly() = default;

  NECTAR_COPY_DEFAULT(IAssembly);

  virtual UInt32 Arch() noexcept { return AssemblyFactory::kArchAMD64; }

  /// =========================================================== ///
  /// @brief compile to object file.
  /// @note Example C++ -> MASM -> AE object.
  /// =========================================================== ///
  virtual Int32 CompileToFormat(STLString src, Int32 arch) = 0;
};

/// =========================================================== ///
/// @brief Number casting unions for different sizes.
/// =========================================================== ///
union NumberCastBase {
  NumberCastBase()  = default;
  ~NumberCastBase() = default;

  static constexpr auto kLimit = 1;

  char   number[kLimit];
  UInt64 raw;
};

union NumberCast64 final {
  NumberCast64() = default;
  explicit NumberCast64(UInt64 raw) : raw(raw) {}

  ~NumberCast64() { raw = 0; }

  static constexpr auto kLimit = 8;

  char   number[kLimit];
  UInt64 raw;
};

union NumberCast32 final {
  NumberCast32() = default;
  explicit NumberCast32(UInt32 raw) : raw(raw) {}

  ~NumberCast32() { raw = 0; }

  static constexpr auto kLimit = 4;

  char   number[kLimit];
  UInt32 raw;
};

union NumberCast16 final {
  NumberCast16() = default;
  explicit NumberCast16(UInt16 raw) : raw(raw) {}

  ~NumberCast16() { raw = 0; }

  static constexpr auto kLimit = 2;

  char   number[kLimit];
  UInt16 raw;
};

union NumberCast8 final {
  NumberCast8() = default;
  explicit NumberCast8(UInt8 raw) : raw(raw) {}

  ~NumberCast8() { raw = 0; }

  char  number;
  UInt8 raw;
};

/// =========================================================== ///
/// @brief Assembly encoder interface.
/// =========================================================== ///
class IAssemblyEncoder {
 public:
  explicit IAssemblyEncoder() = default;
  virtual ~IAssemblyEncoder() = default;

  NECTAR_COPY_DEFAULT(IAssemblyEncoder);

  virtual STLString CheckLine(STLString line, STLString file)                 = 0;
  virtual bool      WriteLine(STLString line, STLString file)                 = 0;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) = 0;
};

/// =========================================================== ///
/// @brief Different architecture encoders.
/// =========================================================== ///

#ifdef __ASM_NEED_AMD64__

class EncoderAMD64 final : public IAssemblyEncoder {
 public:
  explicit EncoderAMD64()  = default;
  ~EncoderAMD64() override = default;

  NECTAR_COPY_DEFAULT(EncoderAMD64);

  virtual STLString CheckLine(STLString line, STLString file) override;
  virtual bool      WriteLine(STLString line, STLString file) override;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) override;

  virtual bool WriteNumber16(const std::size_t& pos, STLString& from_what);
  virtual bool WriteNumber32(const std::size_t& pos, STLString& from_what);
  virtual bool WriteNumber8(const std::size_t& pos, STLString& from_what);
};

#endif  // __ASM_NEED_AMD64__

#ifdef __ASM_NEED_ARM64__

class EncoderARM64 final : public IAssemblyEncoder {
 public:
  explicit EncoderARM64()  = default;
  ~EncoderARM64() override = default;

  NECTAR_COPY_DEFAULT(EncoderARM64);

  virtual STLString CheckLine(STLString line, STLString file) override;
  virtual bool      WriteLine(STLString line, STLString file) override;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) override;
};

#endif  // __ASM_NEED_ARM64__

#ifdef __ASM_NEED_64x0__

class Encoder64x0 final : public IAssemblyEncoder {
 public:
  explicit Encoder64x0()  = default;
  ~Encoder64x0() override = default;

  NECTAR_COPY_DEFAULT(Encoder64x0);

  virtual STLString CheckLine(STLString line, STLString file) override;
  virtual bool      WriteLine(STLString line, STLString file) override;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) override;
};

#endif  // __ASM_NEED_64x0__

#ifdef __ASM_NEED_32x0__

class Encoder32x0 final : public IAssemblyEncoder {
 public:
  explicit Encoder32x0()  = default;
  ~Encoder32x0() override = default;

  NECTAR_COPY_DEFAULT(Encoder32x0);

  virtual STLString CheckLine(STLString line, STLString file) override;
  virtual bool      WriteLine(STLString line, STLString file) override;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) override;
};

#endif  // __ASM_NEED_32x0__

#ifdef __ASM_NEED_PPC__

class EncoderPowerPC final : public IAssemblyEncoder {
 public:
  explicit EncoderPowerPC()  = default;
  ~EncoderPowerPC() override = default;

  NECTAR_COPY_DEFAULT(EncoderPowerPC);

  virtual STLString CheckLine(STLString line, STLString file) override;
  virtual bool      WriteLine(STLString line, STLString file) override;
  virtual bool      WriteNumber(const std::size_t& pos, STLString& from_what) override;
};

#endif  // __ASM_NEED_32x0__
}  // namespace CompilerKit

#endif  // NECTAR_COMPILERKIT_CODEGENERATOR_H
