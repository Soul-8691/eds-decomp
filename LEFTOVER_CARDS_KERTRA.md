# Leftover cards in EDS_Kertra

This lists every card number whose effect is already coded in the game but has no card in the original EDS (see `LEFTOVER_EFFECTS.md`), as the hack `EDS_Kertra.gba` has it: whether the hack already contains the card at its own card number (its "slot", the number its coded effect is attached to), and whether something about it is wrong. Cards the hack does not have are the ones `tools/card_expand/expand_rom.py` adds; their IDs are those of `build/card_expand/kertra_cards.csv` (run with `tools/card_expand/run_expand.sh`).

## Summary

| Status | Cards |
|---|---|
| In its slot: the hack has the card at its own number | 16 |
| In its slot, but the number-to-ID table does not point back to it (bugged) | 1 |
| Another card holds the number (it gets this number's coded effect) | 2 |
| In the hack at another card number (does not get its coded effect) | 1 |
| Not in the hack: added by expand_rom.py | 137 |
| **Total** | **157** |

9 of the cards the hack has carry a note (wrong slot, wrong stats, a changed effect row, a password clash, or a limit of the original code).

## Cards the hack already has

| Number | Card | Kertra | Status | Notes |
|---|---|---|---|---|
| 1213 | Mystic Box | ID 522 | in its slot | OK |
| 1214 | Magic Cylinder | ID 442 | in its slot | OK |
| 1221 | Card Destruction | ID 95 | another card in its slot | number 1221 holds 'Brain Control' (ID 95), which therefore plays with Card Destruction's coded effect; Kertra changed its effect-table row: resolve 0x08039639 -> 0x080353A5, check - -> 0x0802C005, prepare - -> 0x0802E4B1, chainA - -> 0x0802C77D, chainB - -> 0x0803F345 |
| 1232 | Multiply | ID 515 | in its slot | OK |
| 1244 | Shadow Spell | ID 654 | in its slot | password 56830749 (card: 29267084); same password as ID [655] |
| 1246 | Copycat | ID 125 | in its slot | OK |
| 1311 | Burning Land | ID 98 | in its slot | OK |
| 1317 | Shift | ID 657 | in its slot | OK |
| 1322 | Infinite Dismissal | ID 360 | not in its slot | Infinite Dismissal is ID 360 with card number 1323, not 1322, so it does not get the effect coded for 1322; Kertra repointed code: EndPhase_DestroyLowLevelMonsters now tests number 1323; code status Full (passive) |
| 1323 | Gravity Bind | ID 360 | another card in its slot | number 1323 holds 'Infinite Dismissal' (ID 360), which therefore plays with Gravity Bind's coded effect; Kertra repointed code: EndPhase_DestroyLowLevelMonsters (which tested 1322) now tests this number; code status Full (passive) |
| 1325 | Shadow of Eyes | ID 652 | in its slot | OK |
| 1326 | The Legendary Fisherman | ID 742 | in its slot | code status Full (passive) |
| 1423 | Tornado Wall | ID 770 | in its slot | code status Full (passive) |
| 1425 | Torrential Tribute | ID 771 | in its slot | OK |
| 1427 | Infinite Cards | ID 359 | in its slot | code status Full (passive) |
| 1447 | Jar of Greed | ID 368 | in its slot | OK |
| 1449 | United We Stand | ID 794 | in its slot | OK |
| 1450 | Mage Power | ID 441 | in its slot | OK |
| 1513 | Bazoo the Soul-Eater | ID 54 | in its slot, number lookup broken | gCardNumberToId[1513] is 0, not 54: code that looks the card up by number (effects that search, summon or name it) cannot find it |
| 1921 | Kuriboh Token | ID 860 | in its slot | password 00000000 (card: ffffffff) |

## Cards the hack does not have

`expand_rom.py` adds these at their own card numbers. Notes here are limits of the game's original code for that number.

| Number | Card | Expanded ROM | Notes |
|---|---|---|---|
| 1211 | Thousand Knives | added as ID 861 |  |
| 1212 | Dark Magic Curtain | added as ID 862 |  |
| 1220 | Soul Exchange | added as ID 863 | code status Full, buggy: ROM bug: the summon paths tribute the monster in the target side's zone 0, not the target. |
| 1230 | Nightmare's Steelcage | added as ID 864 | code status Full (passive) |
| 1231 | Rocket Warrior | added as ID 865 | code status Full (passive) |
| 1240 | Berfomet | added as ID 866 |  |
| 1241 | Chimera the Flying Mythical Beast | added as ID 867 |  |
| 1242 | Archfiend of Gilfer | added as ID 868 |  |
| 1243 | Ancient Lamp | added as ID 869 | code status Full (passive) |
| 1245 | Scapegoat | added as ID 870 |  |
| 1247 | Mystical Refpanel | added as ID 871 |  |
| 1248 | Makiu, the Magical Mist | added as ID 872 |  |
| 1250 | Swift Gaia the Fierce Knight | added as ID 873 | code status Full (passive) |
| 1251 | Obnoxious Celtic Guard | added as ID 874 | code status Full (passive) |
| 1252 | Fairy King Truesdale | added as ID 875 | code status Full (passive) |
| 1253 | Cipher Soldier | added as ID 876 | code status Full (passive) |
| 1254 | Slate Warrior | added as ID 877 |  |
| 1255 | Electromagnetic Bagworm | added as ID 878 | Kertra changed its effect-table row: resolve 0x08039E31 -> 0x080353A5, check - -> 0x0802C005, prepare - -> 0x0802E4B1, chainA - -> 0x0802C77D, chainB 0x08040D85 -> 0x0803F345 |
| 1256 | Rigorous Reaver | added as ID 879 |  |
| 1257 | Masked Beast Des Gardius | added as ID 880 |  |
| 1258 | The Mask of Remnants | added as ID 881 | Kertra changed its effect-table row: resolve 0x0803A17D -> - |
| 1301 | Michizure | added as ID 882 |  |
| 1302 | Minor Goblin Official | added as ID 883 | code status Full (passive) |
| 1303 | Gamble | added as ID 884 |  |
| 1304 | Attack and Receive | added as ID 885 |  |
| 1305 | Solemn Wishes | added as ID 886 | code status Full (passive) |
| 1306 | Skull Invitation | added as ID 887 | code status Full (passive) |
| 1307 | Bubonic Vermin | added as ID 888 |  |
| 1312 | Cold Wave | added as ID 889 |  |
| 1313 | Fairy Meteor Crush | added as ID 890 |  |
| 1314 | Limiter Removal | added as ID 891 |  |
| 1315 | Rain of Mercy | added as ID 892 |  |
| 1316 | Monster Recovery | added as ID 893 | code status Full (human only): Its target picker has no CPU branch. |
| 1318 | Insect Imitation | added as ID 894 | code status Full, buggy: ROM bug: the packed position is passed as the zone, so the Tribute and level lookup only work for one zone. |
| 1319 | Interdimensional Matter Transporter | added as ID 895 | code status Full (human only): The return is in the Standby Phase code. Its target picker has no CPU branch. |
| 1320 | Ground Collapse | added as ID 896 |  |
| 1321 | Magic Drain | added as ID 897 |  |
| 1324 | Type Zero Magic Crusher | added as ID 898 |  |
| 1327 | Sword Hunter | added as ID 899 | code status Full (passive) |
| 1328 | Drill Bug | added as ID 900 |  |
| 1329 | Deepsea Warrior | added as ID 901 | code status Full (passive) |
| 1330 | Bite Shoes | added as ID 902 |  |
| 1332 | Invitation to a Dark Sleep | added as ID 903 |  |
| 1333 | Thousand-Eyes Idol | added as ID 904 | code status No effect code |
| 1334 | Thousand-Eyes Restrict | added as ID 905 |  |
| 1336 | Hayabusa Knight | added as ID 906 | code status Full (passive) |
| 1337 | Bombardment Beetle | added as ID 907 |  |
| 1338 | 4-Starred Ladybug of Doom | added as ID 908 |  |
| 1339 | Gradius | added as ID 909 | code status No effect code |
| 1340 | Vampire Baby | added as ID 910 | code status Full (passive) |
| 1341 | Mad Sword Beast | added as ID 911 | code status Full (passive) |
| 1344 | Goblin Attack Force | added as ID 912 | code status Full (passive) |
| 1350 | The Fiend Megacyber | added as ID 913 | code status Full (passive) |
| 1351 | Gearfried the Iron Knight | added as ID 914 | code status Full (passive) |
| 1352 | Insect Barrier | added as ID 915 | code status Full (passive) |
| 1402 | Humanoid Slime | added as ID 916 | code status No effect code |
| 1403 | Worm Drake | added as ID 917 | code status No effect code |
| 1404 | Humanoid Worm Drake | added as ID 918 | code status No effect code |
| 1405 | Revival Jam | added as ID 919 |  |
| 1410 | Grand Tiki Elder | added as ID 920 | code status No effect code |
| 1411 | The Masked Beast | added as ID 921 | code status No effect code |
| 1412 | Melchid the Four-Face Beast | added as ID 922 | code status No effect code |
| 1413 | Nuvia the Wicked | added as ID 923 | code status Full (passive) |
| 1414 | Chosen One | added as ID 924 |  |
| 1415 | Mask of Weakness | added as ID 925 |  |
| 1416 | Curse of the Masked Beast | added as ID 926 |  |
| 1417 | Mask of Dispel | added as ID 927 |  |
| 1418 | Mask of Restrict | added as ID 928 | code status Full (passive) |
| 1419 | Mask of the Accursed | added as ID 929 |  |
| 1420 | Mask of Brutality | added as ID 930 |  |
| 1421 | Return of the Doomed | added as ID 931 | code status Partial: The mark sets graveyard flag bit 23, which nothing reads, so the monster never returns. |
| 1422 | Lightning Blade | added as ID 932 | code status Partial: The card text also makes all WATER monsters lose 500 ATK; that part is not coded. |
| 1424 | Fairy Box | added as ID 933 |  |
| 1426 | Jam Breeding Machine | added as ID 934 | code status Full (passive) |
| 1428 | Jam Defender | added as ID 935 |  |
| 1429 | Card of Safe Return | added as ID 936 | code status Full (passive) |
| 1430 | Lady Panther | added as ID 937 |  |
| 1431 | The Unfriendly Amazon | added as ID 938 | code status Full (passive) |
| 1432 | Amazoness Archer | added as ID 939 |  |
| 1433 | Crimson Sentry | added as ID 940 |  |
| 1434 | Fire Princess | added as ID 941 | code status Full (passive) |
| 1435 | Lady Assailant of Flames | added as ID 942 |  |
| 1436 | Fire Sorcerer | added as ID 943 |  |
| 1437 | Spirit of the Breeze | added as ID 944 | code status Full (passive) |
| 1438 | Dancing Fairy | added as ID 945 | code status Full (passive) |
| 1439 | Fairy Guardian | added as ID 946 |  |
| 1441 | Cure Mermaid | added as ID 947 | code status Full (passive) |
| 1442 | Hysteric Fairy | added as ID 948 |  |
| 1444 | The Forgiving Maiden | added as ID 949 |  |
| 1445 | St. Joan | added as ID 950 | code status No effect code |
| 1446 | Darklord Marie | added as ID 951 | code status Full (passive) |
| 1448 | Scroll of Bewitchment | added as ID 952 |  |
| 1451 | Offerings to the Doomed | added as ID 953 |  |
| 1510 | Jowgen the Spiritualist | added as ID 954 |  |
| 1511 | Kycoo the Ghost Destroyer | added as ID 955 |  |
| 1512 | Summoner of Illusions | added as ID 956 | code status Full (human only): For the CPU (player 1) the effect does nothing. |
| 1514 | Dark Necrofear | added as ID 957 |  |
| 1515 | Soul of Purity and Light | added as ID 958 | code status Full (passive) |
| 1516 | Spirit of Flames | added as ID 959 | code status Full (passive) |
| 1517 | Aqua Spirit | added as ID 960 |  |
| 1518 | The Rock Spirit | added as ID 961 | code status Full (passive) |
| 1519 | Garuda the Wind Spirit | added as ID 962 |  |
| 1520 | Gilasaurus | added as ID 963 |  |
| 1521 | Tornado Bird | added as ID 964 |  |
| 1522 | Dreamsprite | added as ID 965 | code status Full (passive) |
| 1523 | Zombyra the Dark | added as ID 966 | code status Full (passive) |
| 1524 | Supply | added as ID 967 |  |
| 1525 | Maryokutai | added as ID 968 |  |
| 1526 | The Last Warrior from Another Planet | added as ID 969 | code status Full (passive) |
| 1527 | Collected Power | added as ID 970 |  |
| 1528 | Destiny Board | added as ID 971 |  |
| 1529 | Dark Spirit of the Silent | added as ID 972 |  |
| 1530 | Royal Command | added as ID 973 |  |
| 1531 | Riryoku Field | added as ID 974 |  |
| 1532 | Skull Lair | added as ID 975 |  |
| 1533 | Graverobber's Retribution | added as ID 976 | code status Full (passive) |
| 1534 | Deal of Phantom | added as ID 977 |  |
| 1535 | Destruction Punch | added as ID 978 |  |
| 1536 | Blind Destruction | added as ID 979 | code status Full (passive) |
| 1537 | The Emperor's Holiday | added as ID 980 |  |
| 1538 | The Dark Door | added as ID 981 | code status Full (passive) |
| 1539 | Spiritualism | added as ID 982 |  |
| 1540 | Cyclon Laser | added as ID 983 |  |
| 1541 | Spirit Message "I" | added as ID 984 |  |
| 1542 | Spirit Message "N" | added as ID 985 |  |
| 1543 | Spirit Message "A" | added as ID 986 |  |
| 1544 | Spirit Message "L" | added as ID 987 |  |
| 1545 | Bait Doll | added as ID 988 |  |
| 1546 | De-Fusion | added as ID 989 | code status Full (human only): For the CPU the effect ends after the return. |
| 1547 | Fusion Gate | added as ID 990 |  |
| 1548 | Ekibyo Drakmord | added as ID 991 |  |
| 1549 | Miracle Dig | added as ID 992 |  |
| 1550 | Dragonic Attack | added as ID 993 |  |
| 1551 | Spirit Elimination | added as ID 994 |  |
| 1552 | Vengeful Bog Spirit | added as ID 995 | code status Full (passive) |
| 1922 | Sheep Token | added as ID 996 |  |
| 1923 | Slime Token | added as ID 997 |  |

## The three numbers without a leftover card

| Number | In the original code | In Kertra |
|---|---|---|
| 230 | an AI hint only | ID 840, 'Wood Clown' |
| 1210 | a second Dark Magician (counts as Dark Magician) | ID 157, 'Dark Magician' (gCardNumberToId[1210] is 0: lookup broken) |
| 1249 | a second Harpie Lady (counts as Harpie Lady) | not used |

## Method

- Read from `EDS_Kertra.gba` (SHA-1 e11277f2) at its own table addresses: names 0x08268AC0, stats 0x0822CC00, ID-to-number 0x0822C4E0, passwords 0x08623080, number-to-ID 0x08623DF4; 861 cards (last ID 860, from Deck Edit's card loop).
- "In its slot" means the number-to-ID entry for the number gives a card with that name, and that card's ID-to-number entry gives the number back. Names are compared exactly, so a card under another spelling counts as not present.
- Stats and passwords are compared with the real card as `expand_rom.py` builds it from YGOPRODeck (type, Spell/Trap kind, ATK, DEF, level, attribute, Normal/Effect/Fusion/Ritual).
- Effect-table rows (0x0819A9D4) and every code pointer to `gCardNumberToId[N]` were compared with the original ROM. Code that tests card numbers as immediate constants was not compared, so a hack edit of that kind would not show here.
- "code status" is the completeness of the original game's code for that number, from `LEFTOVER_EFFECTS.md`.
