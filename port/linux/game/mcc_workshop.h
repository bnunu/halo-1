/* The Steam Workshop item an MCC package came from: its id and title, as the
 * host tells them to a joiner, and the joiner texts built from them. */
#ifndef MCC_WORKSHOP_H
#define MCC_WORKSHOP_H

#define MCC_WORKSHOP_ID_SIZE 24
#define MCC_WORKSHOP_TITLE_SIZE 40
#define MCC_WORKSHOP_SIDECAR "workshop_id.txt"

/* Where the item travels: the bytes of the 128-byte map name after the name's
 * own terminator (names stop at 63 characters). MCC protocol 64 introduces
 * this advisory record; content fingerprints alone still decide admission. */
#define MCC_WORKSHOP_WIRE_OFFSET 64
#define MCC_WORKSHOP_WIRE_SIZE 64
#define MCC_WORKSHOP_WIRE_VERSION 1

struct mcc_workshop_item {
    char id[MCC_WORKSHOP_ID_SIZE];        /* decimal digits; "" when the package is not a Workshop item */
    char title[MCC_WORKSHOP_TITLE_SIZE];  /* printable ASCII; "" when unknown */
};

enum mcc_workshop_refusal {
    MCC_WORKSHOP_NOT_WORKSHOP,   /* the host's map has no item id */
    MCC_WORKSHOP_MISSING,        /* no local package positively identifies the item */
    MCC_WORKSHOP_DIFFERS         /* an identified package lacks the matching level/content */
};

/* A sidecar's text: line 1 the item id (1 to 20 digits), optional line 2 its
 * title. 1 when it holds a valid id. */
int mcc_workshop_parse_sidecar(char const *text, struct mcc_workshop_item *item);
/* A Halo 1 ModInfo.json's text (length bytes): its
 * ModIdentifier.HostedModIds.SteamWorkshopId (a number or a string, not 0)
 * and Title.Neutral. 1 when it holds a valid id. */
int mcc_workshop_parse_modinfo(char const *text, int length, struct mcc_workshop_item *item);
/* The package folder of a canonical name, mcc_maps\<folder>\<level>: 1 and the
 * folder, or 0 for a loose map. */
int mcc_workshop_package_folder(char const *name, char folder[64]);
/* The item a local package belongs to: MCC_WORKSHOP_SIDECAR in the folder,
 * its folder named by the item id (as Steam names it), or the id in its
 * ModInfo.json (mcc_workshop_parse_modinfo). 1 when known. */
int mcc_workshop_local_item(char const *name, struct mcc_workshop_item *item);
/* A level's name made readable: "pat_beavercreek" to "Pat Beavercreek". */
void mcc_workshop_title_from_level(char const *name, char title[MCC_WORKSHOP_TITLE_SIZE]);
/* Put the item into the unused bytes of a map name record (size 128), or
 * clear them when the item has no id. */
void mcc_workshop_wire_pack(char name[128], struct mcc_workshop_item const *item);
/* The item a map name record carries; 1 when it carries a valid one. */
int mcc_workshop_wire_unpack(char const name[128], struct mcc_workshop_item *item);
/* The joiner's text; title is the host's, or the level's readable name. */
void mcc_workshop_refusal_text(enum mcc_workshop_refusal kind, struct mcc_workshop_item const *item,
    char const *level, char *text, unsigned size);
#endif
