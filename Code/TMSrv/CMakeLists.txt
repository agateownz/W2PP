# TMSrv - Town/Map Server
add_executable(TMSrv
    # Common sources
    ${COMMON_SOURCES}
    # TMSrv sources
    CCastleZakum.cpp
    CItem.cpp
    CMob.cpp
    CNPCGene.cpp
    CReadFiles.cpp
    CUser.cpp
    CWarTower.cpp
    GetFunc.cpp
    imple.cpp
    MobKilled.cpp
    ProcessClientMessage.cpp
    ProcessDBMessage.cpp
    ProcessSecMinTimer.cpp
    SendFunc.cpp
    Server.cpp
    _MSG_AcceptParty.cpp
    _MSG_AccountLogin.cpp
    _MSG_AccountSecure.cpp
    _MSG_Action.cpp
    _MSG_ApplyBonus.cpp
    _MSG_Attack.cpp
    _MSG_Buy.cpp
    _MSG_CapsuleInfo.cpp
    _MSG_Challange.cpp
    _MSG_ChallangeConfirm.cpp
    _MSG_ChangeCity.cpp
    _MSG_CharacterLogin.cpp
    _MSG_CharacterLogout.cpp
    _MSG_CombineItem.cpp
    _MSG_CombineItemAgatha.cpp
    _MSG_CombineItemAilyn.cpp
    _MSG_CombineItemAlquimia.cpp
    _MSG_CombineItemEhre.cpp
    _MSG_CombineItemExtracao.cpp
    _MSG_CombineItemLindy.cpp
    _MSG_CombineItemOdin.cpp
    _MSG_CombineItemShany.cpp
    _MSG_CombineItemTiny.cpp
    _MSG_CreateCharacter.cpp
    _MSG_DeleteCharacter.cpp
    _MSG_DeleteItem.cpp
    _MSG_Deposit.cpp
    _MSG_Deprivate.cpp
    _MSG_DropItem.cpp
    _MSG_GetItem.cpp
    _MSG_GuildAlly.cpp
    _MSG_InviteGuild.cpp
    _MSG_MessageChat.cpp
    _MSG_MessageWhisper.cpp
    _MSG_Motion.cpp
    _MSG_NoViewMob.cpp
    _MSG_PKMode.cpp
    _MSG_PutoutSeal.cpp
    _MSG_Quest.cpp
    _MSG_QuitTrade.cpp
    _MSG_REQShopList.cpp
    _MSG_RemoveParty.cpp
    _MSG_ReqBuy.cpp
    _MSG_ReqRanking.cpp
    _MSG_ReqTeleport.cpp
    _MSG_ReqTradeList.cpp
    _MSG_Restart.cpp
    _MSG_Sell.cpp
    _MSG_SendAutoTrade.cpp
    _MSG_SendReqParty.cpp
    _MSG_SetShortSkill.cpp
    _MSG_SplitItem.cpp
    _MSG_Trade.cpp
    _MSG_TradingItem.cpp
    _MSG_UpdateItem.cpp
    _MSG_UseItem.cpp
    _MSG_War.cpp
    _MSG_Withdraw.cpp
    # Resources
    icon1.ico
)

target_include_directories(TMSrv PRIVATE
    ${COMMON_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(TMSrv PRIVATE ${COMMON_LIBS})

# TMSrv output directory (match MSVC layout: Build/Debug/Server/TMSrv/run/)
set_target_properties(TMSrv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Build/Debug/Server/TMSrv/run"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Build/Release/Server/TMSrv/run"
)

# TMSrv compile definitions
target_compile_definitions(TMSrv PRIVATE
    $<$<CONFIG:Debug>:_PACKET_DEBUG>
    WIN32
    _WINDOWS
)

# TMSrv compile options per configuration
target_compile_options(TMSrv PRIVATE
    $<$<CONFIG:Debug>:
        /Od
        /MTd
        /W3
    >
    $<$<CONFIG:Release>:
        /O1
        /MT
        /W3
        /GL
        /Gy    # Function-level linking
        /GF    # String pooling
    >
)
