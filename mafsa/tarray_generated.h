// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TARRAY_H_
#define FLATBUFFERS_GENERATED_TARRAY_H_

#include "flatbuffers/flatbuffers.h"

struct SerialTarray;
struct SerialTarrayBuilder;

struct SerialTarray FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SerialTarrayBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASES = 4,
    VT_CHECKS = 6,
    VT_NEXTS = 8
  };
  const flatbuffers::Vector<uint32_t> *bases() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_BASES);
  }
  const flatbuffers::Vector<int32_t> *checks() const {
    return GetPointer<const flatbuffers::Vector<int32_t> *>(VT_CHECKS);
  }
  const flatbuffers::Vector<int32_t> *nexts() const {
    return GetPointer<const flatbuffers::Vector<int32_t> *>(VT_NEXTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASES) &&
           verifier.VerifyVector(bases()) &&
           VerifyOffset(verifier, VT_CHECKS) &&
           verifier.VerifyVector(checks()) &&
           VerifyOffset(verifier, VT_NEXTS) &&
           verifier.VerifyVector(nexts()) &&
           verifier.EndTable();
  }
};

struct SerialTarrayBuilder {
  typedef SerialTarray Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_bases(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> bases) {
    fbb_.AddOffset(SerialTarray::VT_BASES, bases);
  }
  void add_checks(flatbuffers::Offset<flatbuffers::Vector<int32_t>> checks) {
    fbb_.AddOffset(SerialTarray::VT_CHECKS, checks);
  }
  void add_nexts(flatbuffers::Offset<flatbuffers::Vector<int32_t>> nexts) {
    fbb_.AddOffset(SerialTarray::VT_NEXTS, nexts);
  }
  explicit SerialTarrayBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SerialTarray> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SerialTarray>(end);
    return o;
  }
};

inline flatbuffers::Offset<SerialTarray> CreateSerialTarray(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> bases = 0,
    flatbuffers::Offset<flatbuffers::Vector<int32_t>> checks = 0,
    flatbuffers::Offset<flatbuffers::Vector<int32_t>> nexts = 0) {
  SerialTarrayBuilder builder_(_fbb);
  builder_.add_nexts(nexts);
  builder_.add_checks(checks);
  builder_.add_bases(bases);
  return builder_.Finish();
}

inline flatbuffers::Offset<SerialTarray> CreateSerialTarrayDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint32_t> *bases = nullptr,
    const std::vector<int32_t> *checks = nullptr,
    const std::vector<int32_t> *nexts = nullptr) {
  auto bases__ = bases ? _fbb.CreateVector<uint32_t>(*bases) : 0;
  auto checks__ = checks ? _fbb.CreateVector<int32_t>(*checks) : 0;
  auto nexts__ = nexts ? _fbb.CreateVector<int32_t>(*nexts) : 0;
  return CreateSerialTarray(
      _fbb,
      bases__,
      checks__,
      nexts__);
}

inline const SerialTarray *GetSerialTarray(const void *buf) {
  return flatbuffers::GetRoot<SerialTarray>(buf);
}

inline const SerialTarray *GetSizePrefixedSerialTarray(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<SerialTarray>(buf);
}

inline const char *SerialTarrayIdentifier() {
  return "TDIC";
}

inline bool SerialTarrayBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, SerialTarrayIdentifier());
}

inline bool VerifySerialTarrayBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<SerialTarray>(SerialTarrayIdentifier());
}

inline bool VerifySizePrefixedSerialTarrayBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<SerialTarray>(SerialTarrayIdentifier());
}

inline const char *SerialTarrayExtension() {
  return "tdic";
}

inline void FinishSerialTarrayBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SerialTarray> root) {
  fbb.Finish(root, SerialTarrayIdentifier());
}

inline void FinishSizePrefixedSerialTarrayBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SerialTarray> root) {
  fbb.FinishSizePrefixed(root, SerialTarrayIdentifier());
}

#endif  // FLATBUFFERS_GENERATED_TARRAY_H_