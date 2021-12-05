#include "fonts.h"

#include "YanoneKaffeesatz-SemiBold13pt8b.h"
#include "YanoneKaffeesatz-SemiBold11pt8b.h"
#include "YanoneKaffeesatz-SemiBold10pt8b.h"
#include "YanoneKaffeesatz-SemiBold9pt8b.h"
#include "meteocons15pt8b.h"
#include "meteocons20pt8b.h"
#include "moon_phases15pt8b.h"

const GFXfont * fnt_YanoneSB13() {
    return &YanoneKaffeesatz_SemiBold13pt8b;
}  

// VarovaniChmi
const GFXfont * fnt_YanoneSB11() {
    return &YanoneKaffeesatz_SemiBold11pt8b;
}  

const GFXfont * fnt_YanoneSB10() {
    return &YanoneKaffeesatz_SemiBold10pt8b;
}  

const GFXfont * fnt_YanoneSB9() {
    return &YanoneKaffeesatz_SemiBold9pt8b;
}  

// InfoMesic
const GFXfont * fnt_MoonPhases15() {
    return &moon_phases15pt8b;
}  

// VarovaniChmi
const GFXfont * fnt_Meteocons15() {
    return &meteocons15pt8b;
}  

// InfoSlunce
const GFXfont * fnt_Meteocons20() {
    return &meteocons20pt8b;
}  
