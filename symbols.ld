/* Hand-named absolute symbols (RAM variables, IO, ...). Format: name = 0xADDRESS; */
IntrTable = 0x03000000;
gMain = 0x03000040;
gIntrMainBuf = 0x0300004C;
gMain_intrCheck = 0x0300044C;
gMain_callback = 0x03000450;
gMain_oamBuffer = 0x03004470;
gBgTileBuffer = 0x02010014;
gScratchBuffer = 0x02030000;
gDeckEditCardLists = 0x0202A000;
gDeckEditHeldBack = 0x0202C200;
/* Mod variables in free EWRAM (below gScratchBuffer): Injection Fairy Lily's battle state (src/added_effects.c). */
gInjectionFairyLily = 0x0202FF00;
gModTestingFreezeOpponent = 0x0202FF03;
/* Mod: a CPU test scenario's card under test, the only card the CPU may activate (src/mod_testing_automation.c;
   free RAM after gModLink, which ends by 0x0202D800). */
gModTestingOnlyCard = 0x0202D800;
gModTestingSelect = 0x0202D802;
gModTestingFlags2 = 0x0202D804;
gModTestingLastShown = 0x0202D806;
gModDuel = 0x0202FF04;
/* Mod: the Synchro Summon state of src/mod_synchro.c (at most 0x100 bytes, before gInjectionFairyLily). */
gModSynchro = 0x0202FE00;
/* Mod: added cards' duel state that gModDuel has no room for (struct ModDuelExtra, src/added_effects.c), after
   gModSynchro (0xB0 bytes). */
gModDuelExtra = 0x0202FEB0;
/* Mod: Solomon's deck, the recipe the player picked (src/mod_decks.c; free RAM after gModDeckRing). */
gModOpponentRecipe = 0x0202F400;
/* Mod: the player's deck against Debug while Debug's is picked (src/mod_recipes.c; after gModOpponentRecipe). */
gModPlayerDeckStash = 0x0202F500;
/* Mod: the format warning of src/mod_banlist.c (free RAM after the Deck Edit lists, which end by 0x0202D200). */
gBanlistView = 0x0202E000;
/* Mod: the Deck Recipes scene of src/mod_recipes.c (after gBanlistView, which ends by 0x0202E304). */
gDeckRecipes = 0x0202E400;
/* Mod: the Booster Packs scene of src/mod_packs.c (after gDeckRecipes, which ends by 0x0202EC40). */
gPackShop = 0x0202F000;
/* Mod: the pick-a-pack list's rows and a pack's info of src/mod_format_packs.c (0xCC bytes, after gPackShop). */
gModPackList = 0x0202F010;
/* Mod: the DP lines of src/mod_dp.c. */
gModDP = 0x0202F100;
/* Mod: the Banlists scene of src/mod_banlists.c, and the main menu's scroll (main_menu.c). */
gBanlistsMenu = 0x0202F200;
gMainMenuScroll = 0x0202F260;
gSettingsMenu = 0x0202F268;
/* Mod: the card sprite slot cache of src/mod_field_icons.c (free RAM, 0x0202D000-0x0202FFFF). */
gModCardSprites = 0x0202F300;
/* Mod: the cards of Deck Edit's ring (src/mod_field_icons.c). */
gModDeckRing = 0x0202F360;
/* Mod: the best-of-3 match state of src/mod_decks.c (the CPU's deck, the player's Deck and Side Deck; 0xA6 bytes),
 * in the free RAM after the Deck Edit lists. */
gModMatch = 0x0202D400;
/* Mod: the Xyz Materials and the Xyz Summon state of src/mod_xyz.c (at most 0x200 bytes, after gModMatch). */
gModXyz = 0x0202D500;
/* Mod: the Extra Monster Zones, the zone pick and the Link Summon state of src/mod_link.c (at most 0x100 bytes). */
gModLink = 0x0202D700;
/* Mod: a Link Monster's card frame with its arrows (src/mod_link.c ModLinkFrame, 0x2698 bytes), inside
 * gBustupBitmapBuffer (0x02031014-0x02036A14), which only the dialogue scenes and the pre-duel picture use. */
gModLinkFrame = 0x02034000;
gTextCanvas = 0x02000000;
gSaveData = 0x02011C20;
gSaveDataSignature = 0x02013D86;
gSoundDriver = 0x03005210;
gSoundSeTracks = 0x03005308;
gSoundPcmChannels = 0x030053AC;
gSoundDmaPos = 0x0300540C;
gSoundPcmBuffer = 0x03005414;
gSoundMixCodeRam = 0x03005A54;
gLinkBuf = 0x030049D0;
gLinkSio = 0x03005B60;
gLinkSioTxPayload = 0x03005B6A;
gSioMultiRecv = 0x03006644;
gTitleState = 0x0201527C;
gMainMenuCursor = 0x02015ED8;
gRecordScreen = 0x0201F814;
gBustup = 0x02013DE0;
gBustupCursor = 0x020147B4;
gBustupSprites = 0x02014888;
gBustupMouthAnim = 0x02014EB4;
gBustupFade = 0x020150CC;
gBustupBitmapBuffer = 0x02031014;
gBustupBoxBitmap = 0x0203A614;
gCalendar = 0x0201F7D0;
gOpponentSelect = 0x0201F7E0;
gCardDetail = 0x02013D90;
gDeckEdit = 0x0201DB20;
gPackOpenWork = 0x02015160;
gCardTrading = 0x0201F780;
gPassword = 0x0201F7B0;
gDuel = 0x020192E0;
gDuelPlayers = 0x020192E4;
gDuelZones = 0x0201930C;
gDuelSpellTrapZones = 0x020195F0;
gDuelFieldZone = 0x020198D4;
gDuelHands = 0x02019968;
gDuelDecks = 0x02019AA8;
gDuelGraveyards = 0x02019BE8;
gDuelFusionDecks = 0x02019D28;
gDuelBanished = 0x02019E68;
gDuelBanishedInfo = 0x02019FA8;
gDuelZonesP1 = 0x0201A070;
gDuelHandP1 = 0x0201A6CC;
gDuelDeckP1 = 0x0201A80C;
gDuelCmd = 0x020185C0;
gDuelCmdCard = 0x02018DD4;
gDuelCtrl = 0x02015EE8;
gChain = 0x02017A40;
gChainEffectWork = 0x02017E20;
gEventResponseEntry = 0x02017EE8;
gChainQueryEntry = 0x02017EFC;
gChainQueryEntryRaw = 0x02017F10;
gChainProxyLink = 0x02017F24;
gBattle = 0x02018450;
gSummonAction = 0x0201CF90;
gAiState = 0x02015EF0;
gAiWork = 0x02015F00;
gBattleScene = 0x020185A4;
gChainListScreen = 0x020185B8;
gTextBox = 0x0201AE60;
gDuelScreen = 0x0201CFB0;
gDuelTextTiles = 0x0201CFB8;
gDuelAnimArgs = 0x0201D7E4;
gCardListView = 0x0201D810;
gCardListViewCards = 0x0201D81C;
gCoinTossWork = 0x02015280;
gCoinTossSparkles = 0x02015DD0;
gDuelScene = 0x02017A30;
gDiceScreen = 0x0201F820;
gSceneWork = 0x02020310;
gFinalLettersAnim = 0x02020C64;
gLinkState = 0x02017FB0;
gLinkRxCards = 0x02017FB4;
gLinkTxCards = 0x020181B8;
gLinkRemoteEntries = 0x0201840C;
gDeckEditFade = 0x0201E138;
gDeckEditListPos = 0x0201E140;
gDeckEditScrollEase = 0x0201E148;
gDeckEditTextFlags = 0x0201E160;
gDeckEditListRow = 0x0201EFC0;
gDeckEditSortScratch = 0x0201EFC4;
gDeckEditObjAffine = 0x0201F3D0;
gDeckEditFrameSlots = 0x0201F6D8;
gDeckEditCurList = 0x0201F73C;
gDeckEditCardMove = 0x0201F740;
gDeckEditPanelAlpha = 0x0201F770;
gDeckEditNameIndexCache = 0x0201F775;
gDuelHandCountP1 = 0x0201A04A;
gDuelNegationFlags = 0x0201ADAD;
gDuelTurnFlags = 0x0201ADF2;
gDuelBattleStep = 0x0201ADF6;
gDuelPromptResult = 0x0201AE44;
gChainLinks = 0x02017CC0;
gChainEffectCounter = 0x02017E22;
gChainEffectCard = 0x02017E28;
gChainDeckReorderCursor = 0x02017F7C;
gChainEffectCards = 0x02017F84;
gBgMaps = 0x0300045C;
gBgMap3 = 0x03001C5C;
gLinkSendBuf = 0x03005204;
gLinkSioRxWork = 0x03006598;
gSaveDataDeckSize = 0x02013CE8;
gSaveDataFusionDeckSize = 0x02013CEC;
gPackListPackCount = 0x0202037C;
