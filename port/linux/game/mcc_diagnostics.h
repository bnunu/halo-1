/* Load diagnostics belong to one MCC runtime and are cleared on unload. */
#ifndef MCC_DIAGNOSTICS_H
#define MCC_DIAGNOSTICS_H
#include <stdint.h>
struct mcc_runtime;
struct mcc_diagnostic {
    uint32_t handle;
    char tag[320];
    char location[192];
};
void mcc_diagnostic_tag(struct mcc_runtime *runtime, uint32_t handle);
void mcc_diagnostic_at(struct mcc_runtime *runtime, char const *format, ...);
int mcc_diagnostic_fail(struct mcc_runtime *runtime, char const *format, ...);
#endif
