SET(SOURCES screens/ui_TouchScreen.c
    screens/ui_DispScreen.c
    screens/ui_Disp2Screen.c
    screens/ui_LoudScreen.c
    screens/ui_MicScreen.c
    screens/ui_Sensor1Screen.c
    screens/ui_Sensor2Screen.c
    screens/ui_FinalScreen.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_loudspeaker_png.c
    images/ui_img_mic_png.c
    fonts/ui_font_Heiti14.c
    fonts/ui_font_Heiti16.c
    fonts/ui_font_Heiti20.c)

add_library(ui ${SOURCES})
