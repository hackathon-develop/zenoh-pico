#include "stdbool.h"
#include "string.h"
bool _zp_ke_includes_stardsl_chunk(char const *lstart, const char *lend, char const *rstart, const char *rend) {
    while (lstart < lend && rstart < rend) {
        char l = *lstart++;
        char r = *rstart++;
        if (l == '$') {
            if (++lstart == lend) {
                return true;
            }
            return _zp_ke_includes_stardsl_chunk(lstart, lend, rstart - 1, rend) ||
                   _zp_ke_includes_stardsl_chunk(lstart - 2, lend, rstart, rend);
        } else if (l != r) {
            return false;
        }
    }
    return (lstart == lend && rstart == rend) || (lend - lstart == 2 && lstart[0] == '$');
}

bool _zp_ke_includes_stardsl(char const *lstart, const size_t llen, char const *rstart, const size_t rlen) {
    size_t lclen;
    bool streq;
    const char *lend = lstart + llen;
    const char *rend = rstart + rlen;
    for (;;) {
        const char *lns = memchr(lstart, '/', lend - lstart);
        const char *lcend = lns ? lns : lend;
        const char *rns = memchr(rstart, '/', rend - rstart);
        const char *rcend = rns ? rns : rend;
        char lwildness = 0;
        char rwildness = 0;
        if (*lstart == '*') {
            lwildness = lcend - lstart;
        }
        if (*rstart == '*') {
            rwildness = rcend - rstart;
        }
        if (rwildness > lwildness) {
            return false;
        }
        switch (lwildness) {
            case 2:
                return !lns || _zp_ke_includes_stardsl(lns + 1, lend - (lns + 1), rstart, rend - rstart) ||
                       (rns && _zp_ke_includes_stardsl(lstart, lend - lstart, rns + 1, rend - (rns + 1)));
            case 1:
                break;  // if either chunk is a small wild, yet neither is a big wild, just skip this chunk inspection
            default:
                lclen = lcend - lstart;
                streq = (lclen == rcend - rstart) && (strncmp(lstart, rstart, lclen) == 0);
                if (!streq && !_zp_ke_includes_stardsl_chunk(lstart, lcend, rstart, rcend)) {
                    return false;
                }
        }
        if (!lns || !rns) {
            return lns == rns;
        }
        lstart = lns + 1;
        rstart = rns + 1;
    }
}
bool _zp_ke_includes_nodsl(char const *lstart, const size_t llen, char const *rstart, const size_t rlen) {
    const char *lend = lstart + llen;
    const char *rend = rstart + rlen;
    for (;;) {
        const char *lns = memchr(lstart, '/', lend - lstart);
        const char *lcend = lns ? lns : lend;
        const char *rns = memchr(rstart, '/', rend - rstart);
        const char *rcend = rns ? rns : rend;
        char lwildness = 0;
        char rwildness = 0;
        if (*lstart == '*') {
            lwildness = lcend - lstart;
        }
        if (*rstart == '*') {
            rwildness = rcend - rstart;
        }
        if (rwildness > lwildness) {
            return false;
        }
        switch (lwildness) {
            case 2:
                return !lns || _zp_ke_includes_nodsl(lns + 1, lend - (lns + 1), rstart, rend - rstart) ||
                       (rns && _zp_ke_includes_nodsl(lstart, lend - lstart, rns + 1, rend - (rns + 1)));
            case 1:
                break;  // if either chunk is a small wild, yet neither is a big wild, just skip this chunk inspection
            default:
                if ((lcend - lstart != rcend - rstart) || strncmp(lstart, rstart, lcend - lstart)) {
                    return false;
                }
        }
        if (!lns || !rns) {
            return lns == rns;
        }
        lstart = lns + 1;
        rstart = rns + 1;
    }
}

bool _zp_ke_includes(const char *lstart, const size_t llen, const char *rstart, const size_t rlen) {
    bool streq = (llen == rlen) && (strncmp(lstart, rstart, llen) == 0);
    if (streq) {
        return true;
    }
    int contains = 0;  // STAR: 1, DSL: 2
    char const *end = lstart + llen;
    for (char const *c = lstart; c < end; c++) {
        if (*c == '*') {
            contains |= 1;
        }
        if (*c == '$') {
            contains |= 3;
            break;
        }
    }
    end = rstart + rlen;
    for (char const *c = rstart; contains < 3 && c < end; c++) {
        if (*c == '*') {
            contains |= 1;
        }
        if (*c == '$') {
            contains |= 3;
        }
    }
    switch (contains) {
        case 0:
            return false;
        case 1:
            return _zp_ke_includes_nodsl(lstart, llen, rstart, rlen);
        default:
            return _zp_ke_includes_stardsl(lstart, llen, rstart, rlen);
    }
}

#ifdef QUICKTEST
#include "assert.h"
#include "stdio.h"
bool includes(const char *l, const char *r) { return _zp_ke_includes(l, strlen(l), r, strlen(r)); }
int main() {
    assert(includes("a", "a") == true);
    assert(includes("a/b", "a/b") == true);
    assert(includes("*", "a") == true);
    assert(includes("a", "*") == false);
    assert(includes("*", "aaaaa") == true);
    assert(includes("**", "a") == true);
    assert(includes("a", "**") == false);
    assert(includes("**", "a") == true);
    assert(includes("**", "a/a/a/a") == true);
    assert(includes("**", "*/**") == true);
    assert(includes("*/**", "*/**") == true);
    assert(includes("*/**", "**") == false);
    assert(includes("a/a/a/a", "**") == false);
    assert(includes("a/*", "a/b") == true);
    assert(includes("a/*/b", "a/b") == false);
    assert(includes("a/**/b", "a/b") == true);
    assert(includes("a/b$*", "a/b") == true);
    assert(includes("a/b", "a/b$*") == false);
    assert(includes("a/$*b$*", "a/b") == true);
    assert(includes("a/$*b", "a/b") == true);
    assert(includes("a/b$*", "a/bc") == true);
    assert(includes("a/$*b$*", "a/ebc") == true);
    assert(includes("a/$*b", "a/cb") == true);
    assert(includes("a/b$*", "a/ebc") == false);
    assert(includes("a/$*b", "a/cbc") == false);
    assert(includes("a/**/b$*", "a/b") == true);
    assert(includes("a/**/$*b$*", "a/b") == true);
    assert(includes("a/**/$*b", "a/b") == true);
    assert(includes("a/**/b$*", "a/bc") == true);
    assert(includes("a/**/$*b$*", "a/ebc") == true);
    assert(includes("a/**/$*b", "a/cb") == true);
    assert(includes("a/**/b$*", "a/ebc") == false);
    assert(includes("a/**/$*b", "a/cbc") == false);
    return 0;
}
#endif