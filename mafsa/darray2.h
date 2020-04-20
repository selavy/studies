#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <iosfwd>


struct Darray2
{
    using u32 = uint32_t;

    std::vector<u32> bases;
    std::vector<int> checks;

    Darray2();
    void trim();
    void insert(const char* const word);
    void insert(const std::string& word) { return insert(word.c_str()); }
    bool isword(const char* const word)  const;
    bool isword(const std::string& word) const { return isword(word.c_str()); }

    // TODO(peter): maybe move this to a "serializers.h"?
    static std::optional<Darray2> deserialize(const std::string& filename);

    void dump_stats(std::ostream& os) const;

    void dumpstate() const;

private:
    int  base(int index) const;
    int  check(int index) const;
    bool term(int index) const;
    void setbase(int index, int val, bool term);
    void setbase(int index, int val);
    void setcheck(int index, int val);
    void setterm(int index, bool val);
    void clrbase(int index);
    void clrcheck(int index);

    void relocate(int s, int b, int* childs, int n_childs);
    int  countchildren(int s, int* childs) const;
    static int findbaserange(const int* const first, const int* const last, const int* const cs, const int* const csend);

    static constexpr int MIN_CHILD_OFFSET = 1;
    static constexpr int MAX_CHILD_OFFSET = 27;
    static constexpr int MAX_BASE    = (1 << 30) - MAX_CHILD_OFFSET;
    static constexpr u32 UNSET_BASE  =  0;
    static constexpr int UNSET_CHECK = -1;
};
