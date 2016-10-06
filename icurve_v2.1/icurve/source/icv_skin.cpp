#include <QString>
#include "icv_skin.h"

QString globalCss =
"QMenu {                                                                  "
"     background-color: #F5F5F5;                                          "
"     margin: 5px;                                                        "
" }                                                                       "
" QMenu::item {                                                           "
"     padding: 2px 25px 2px 20px;                                         "
"     border: 1px solid transparent;                                      "
" }                                                                       "
"                                                                         "
" QMenu::item:selected {                                                  "
"     border-color: #7bbfea;                                              "
"     background:#7bbfea;                                                 "
" }                                                                       "
"                                                                         "
" QMenu::icon:checked {                                                   "
"     background: gray;                                                   "
"     border: 1px inset gray;                                             "
"     position: absolute;                                                 "
"     top: 1px;                                                           "
"     right: 1px;                                                         "
"     bottom: 1px;                                                        "
"     left: 1px;                                                          "
" }                                                                       "
"                                                                         "
" QMenu::separator {                                                      "
"     height: 1px;                                                        "
"     background: #d3d7d4;                                                "
"     margin-left: 10px;                                                  "
"     margin-right: 5px;                                                  "
" }                                                                       "
"                                                                         "
" QMenu::indicator {                                                      "
"     width: 13px;                                                        "
"     height: 13px;                                                       "
" }                                                                       "
"                                                                         "
"                                                                         "
" QToolBar {                                                              "
"     background-color: #F5F5F5;                                          "
" }                                                                       "
"                                                                         "
"QMainWindow{                                                             "
"     background-color: #F5F5F5;                                          "
"     background-image: url(:/icurve/images/bg.png);                      "
"     background-repeat: repeat-xy;                                       "
" }                                                                       "
"                                                                         "
"                                                                         "
"QDialog{                                                                 "
"   background-image: url(bg.png);                                        "
"   border: 1px solid #999;                                               "
"   border-radius: 5px;                                                   "
"}                                                                        "
"                                                                         "
"QPushButton{                                                             "
"        /*                                                               "
"          color:white;                                                   "
"          border-radius:8px;                                             "
"          background-color: black;                                       "
"        */                                                               "
"}                                                                        "
"QLabel{                                                                  "
"   color:black;                                                          "
"}                                                                        "
"                                                                         "
"QLineEdit{                                                               "
"         text-align: center;                                             "
"        /*                                                               "
"         color:red;                                                      "
"         background-color: transparent;                                  "
"         border: 1px solid white;                                        "
"         selection-color:yellow;                                         "
"         selection-background-color:green;                               "
"        */                                                               "
"}                                                                        "
"QRadioButton{                                                            "
"   color: black;                                                         "
"}                                                                        "
"QRadioButton::indicator:on{                                              "
"   background-image: url(1.gif);                                         " 
"}                                                                        "
"QComboBox{                                                               "
"      /*                                                                 "
"         min-width: 4em;                                                 "
"         background-color: white;                                        "
"         border: 1px solid black;                                        "
"      */                                                                 "
"}                                                                        "
"QComboBox:on{                                                            "
"   color:black;                                                          "
"}                                                                        "
"QComboBox:off{                                                           "
"   color:black;                                                          "
"}                                                                        "
"QComboBox QAbstractItemView {                                            "
"    border: 1px solid black;                                             "
"    color:black;                                                         "
"    selection-color: black;                                              "
"    selection-background-color: #7bbfea;                                 "
"}                                                                        "
"                                                                         "
"QProgressBar{border: 1px solid black;                                    "
"   border: 1px solid #999;                                               "
"   border-radius:5px;                                                    "
"   text-align: center;                                                   "
"   background-color: transparent;                                        "
"}                                                                        "
"                                                                         "
"QProgressBar::chunk {                                                    "
"    background-color: #33a3dc;                                           "
"    width: 10px;                                                         "
"}                                                                        ";


IcvSkin::IcvSkin(void)
{
    skinCss = globalCss;
}


IcvSkin::~IcvSkin(void)
{

}


QString IcvSkin::GetSkinCss(void)
{
    return skinCss;
}
