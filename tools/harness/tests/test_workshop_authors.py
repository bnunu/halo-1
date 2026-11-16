"""The Steam Workshop items' authors (port/linux/src/workshop_authors.c): the creator read from Steam's details
answer for the item asked about only, a profile's name read and cleaned (CDATA or escaped, no control characters or
broken UTF-8, cut between characters), and the items asked about once, again only 10 minutes after Steam could not
say, and never replaced while being asked about. Steam is the test's: nothing here reaches it."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, mutated, read, run  # noqa: E402

CASES = [
    "creator", "creator-other-item", "creator-refused", "name-cdata", "name-escaped", "name-controls", "name-invalid",
    "name-long",
    "name-missing", "ask", "retry", "ids-refused", "full-while-asking",
]

JSMN = '#include "../../third_party/jsmn/jsmn.h"'

# one thread's pthread.h: no locks needed, and no thread started (the case asks in its stead). Its types have names
# of their own: the C library's headers (glibc's) may have declared the real ones already
PTHREAD = """typedef int test_pthread_mutex_t, test_pthread_cond_t, test_pthread_t;
#define pthread_mutex_t test_pthread_mutex_t
#define pthread_cond_t test_pthread_cond_t
#define pthread_t test_pthread_t
#define PTHREAD_MUTEX_INITIALIZER 0
#define PTHREAD_COND_INITIALIZER 0
#define pthread_mutex_lock(lock) ((void)(lock))
#define pthread_mutex_unlock(lock) ((void)(lock))
#define pthread_cond_wait(cond, lock) ((void)(cond), (void)(lock))
#define pthread_cond_signal(cond) ((void)(cond), test_signals++)
#define pthread_create(thread, attributes, start, argument) ((void)(thread), (void)(start), test_threads_started++, 1)
#define pthread_detach(thread) ((void)(thread))
"""

# a fault in workshop_authors.c and the case that must catch it
NEGATIVE_CONTROLS = {
    "any-items-creator": (("sizeof(id)) || strcmp(id, item_id))", "sizeof(id)))"), "creator-other-item"),
    "creator-unchecked": (("creator, 24) &&\n\t\t\tworkshop_authors_decimal(creator, 20);", "creator, 24);"),
                          "creator-refused"),
    "controls-kept": (("|| first < 0x20 || first == 0x7F)", ")"), "name-controls"),
    "broken-characters-kept": (("if (!size || taken < size ||", "if ("), "name-controls"),
    "overlong-kept": (("value < smallest[size] || ", ""), "name-invalid"),
    "surrogates-kept": (("(value >= 0xD800 && value <= 0xDFFF) || ", ""), "name-invalid"),
    "past-unicode-kept": ((" || value > 0x10FFFF))", "))"), "name-invalid"),
    "c1-controls-kept": (("{ 0, 0, 0xA0, 0x800, 0x10000 }", "{ 0, 0, 0x80, 0x800, 0x10000 }"), "name-invalid"),
    "entities-unread": (("plain && first == '&' &&", "0 &&"), "name-escaped"),
    "ends-untrimmed": (("\twhile (used && name[used - 1] == ' ')\n\t\tused--;\n", ""), "name-cdata"),
    "retried-at-once": (("time(NULL) - authors.items[index].failed_at >= RETRY_SECONDS", "1"), "retry"),
    "retried-never": (("\t\tauthors.items[index].state = _item_wanted;\n\t\tpthread_cond_signal(&authors.wake);\n\t}\n\tif",
                       "\t}\n\tif"), "retry"),
    "asking-replaced": (("\t\t\tif (authors.items[authors.next].state == _item_asking)\n"
                         "\t\t\t\tauthors.next = (authors.next + 1) % MAXIMUM_ITEMS;\n", ""), "full-while-asking"),
    "bad-ids-asked": (("!item_id || size < 1 || !workshop_authors_decimal(item_id, 20)", "!item_id || size < 1"),
                      "ids-refused"),
}


def generated(fault=None):
    code = read("port/linux/src/workshop_authors.c")
    if fault:
        code = mutated(code, *fault)
    json = mutated(read("port/linux/src/server_json.c"), JSMN, '#include "jsmn.h"')
    return (("workshop_authors.h", read("port/linux/src/workshop_authors.h")),
            ("server_json.h", read("port/linux/src/server_json.h")), ("jsmn.h", read("port/third_party/jsmn/jsmn.h")),
            ("update.h", read("port/linux/src/update.h")), ("pthread.h", PTHREAD),
            ("under_test.inc", json + "\n" + code))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("workshop_authors", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("workshop_authors", generated(fault)), case)
    assert status == CHECK_FAILED, f"workshop_authors.c with a fault ({control}) passed '{case}': the test cannot see it"
