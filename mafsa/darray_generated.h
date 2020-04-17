// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DARRAY_H_
#define FLATBUFFERS_GENERATED_DARRAY_H_

#include "flatbuffers/flatbuffers.h"

struct SerialDarray;
struct SerialDarrayBuilder;

struct SerialDarray FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SerialDarrayBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASES = 4,
    VT_CHECKS = 6
  };
  const flatbuffers::Vector<uint32_t> *bases() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_BASES);
  }
  const flatbuffers::Vector<int32_t> *checks() const {
    return GetPointer<const flatbuffers::Vector<int32_t> *>(VT_CHECKS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASES) &&
           verifier.VerifyVector(bases()) &&
           VerifyOffset(verifier, VT_CHECKS) &&
           verifier.VerifyVector(checks()) &&
           verifier.EndTable();
  }
};

struct SerialDarrayBuilder {
  typedef SerialDarray Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_bases(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> bases) {
    fbb_.AddOffset(SerialDarray::VT_BASES, bases);
  }
  void add_checks(flatbuffers::Offset<flatbuffers::Vector<int32_t>> checks) {
    fbb_.AddOffset(SerialDarray::VT_CHECKS, checks);
  }
  explicit SerialDarrayBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SerialDarray> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SerialDarray>(end);
    return o;
  }
};

inline flatbuffers::Offset<SerialDarray> CreateSerialDarray(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> bases = 0,
    flatbuffers::Offset<flatbuffers::Vector<int32_t>> checks = 0) {
  SerialDarrayBuilder builder_(_fbb);
  builder_.add_checks(checks);
  builder_.add_bases(bases);
  return builder_.Finish();
}

inline flatbuffers::Offset<SerialDarray> CreateSerialDarrayDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint32_t> *bases = nullptr,
    const std::vector<int32_t> *checks = nullptr) {
  auto bases__ = bases ? _fbb.CreateVector<uint32_t>(*bases) : 0;
  auto checks__ = checks ? _fbb.CreateVector<int32_t>(*checks) : 0;
  return CreateSerialDarray(
      _fbb,
      bases__,
      checks__);
}

inline const SerialDarray *GetSerialDarray(const void *buf) {
  return flatbuffers::GetRoot<SerialDarray>(buf);
}

inline const SerialDarray *GetSizePrefixedSerialDarray(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<SerialDarray>(buf);
}

inline const char *SerialDarrayIdentifier() {
  return "DDIC";
}

inline bool SerialDarrayBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, SerialDarrayIdentifier());
}

inline bool VerifySerialDarrayBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<SerialDarray>(SerialDarrayIdentifier());
}

inline bool VerifySizePrefixedSerialDarrayBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<SerialDarray>(SerialDarrayIdentifier());
}

inline const char *SerialDarrayExtension() {
  return "ddic";
}

inline void FinishSerialDarrayBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SerialDarray> root) {
  fbb.Finish(root, SerialDarrayIdentifier());
}

inline void FinishSizePrefixedSerialDarrayBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SerialDarray> root) {
  fbb.FinishSizePrefixed(root, SerialDarrayIdentifier());
}

#endif  // FLATBUFFERS_GENERATED_DARRAY_H_
