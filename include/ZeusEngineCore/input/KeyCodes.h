#pragma once

namespace ZEN {

    namespace Key {

        using KeyCode = int;

        static constexpr KeyCode Space      = 32;
        static constexpr KeyCode Apostrophe = 39;  /* ' */
        static constexpr KeyCode Comma      = 44;  /* , */
        static constexpr KeyCode Minus      = 45;  /* - */
        static constexpr KeyCode Period     = 46;  /* . */
        static constexpr KeyCode Slash      = 47;  /* / */

        static constexpr KeyCode D0 = 48;
        static constexpr KeyCode D1 = 49;
        static constexpr KeyCode D2 = 50;
        static constexpr KeyCode D3 = 51;
        static constexpr KeyCode D4 = 52;
        static constexpr KeyCode D5 = 53;
        static constexpr KeyCode D6 = 54;
        static constexpr KeyCode D7 = 55;
        static constexpr KeyCode D8 = 56;
        static constexpr KeyCode D9 = 57;

        static constexpr KeyCode Semicolon = 59;  /* ; */
        static constexpr KeyCode Equal     = 61;  /* = */

        static constexpr KeyCode A = 65;
        static constexpr KeyCode B = 66;
        static constexpr KeyCode C = 67;
        static constexpr KeyCode D = 68;
        static constexpr KeyCode E = 69;
        static constexpr KeyCode F = 70;
        static constexpr KeyCode G = 71;
        static constexpr KeyCode H = 72;
        static constexpr KeyCode I = 73;
        static constexpr KeyCode J = 74;
        static constexpr KeyCode K = 75;
        static constexpr KeyCode L = 76;
        static constexpr KeyCode M = 77;
        static constexpr KeyCode N = 78;
        static constexpr KeyCode O = 79;
        static constexpr KeyCode P = 80;
        static constexpr KeyCode Q = 81;
        static constexpr KeyCode R = 82;
        static constexpr KeyCode S = 83;
        static constexpr KeyCode T = 84;
        static constexpr KeyCode U = 85;
        static constexpr KeyCode V = 86;
        static constexpr KeyCode W = 87;
        static constexpr KeyCode X = 88;
        static constexpr KeyCode Y = 89;
        static constexpr KeyCode Z = 90;

        static constexpr KeyCode Escape      = 256;
        static constexpr KeyCode Enter       = 257;
        static constexpr KeyCode Tab         = 258;
        static constexpr KeyCode Backspace   = 259;
        static constexpr KeyCode Insert      = 260;
        static constexpr KeyCode Delete      = 261;

        static constexpr KeyCode Right       = 262;
        static constexpr KeyCode Left        = 263;
        static constexpr KeyCode Down        = 264;
        static constexpr KeyCode Up          = 265;

        static constexpr KeyCode PageUp      = 266;
        static constexpr KeyCode PageDown    = 267;
        static constexpr KeyCode Home        = 268;
        static constexpr KeyCode End         = 269;

        static constexpr KeyCode CapsLock    = 280;
        static constexpr KeyCode ScrollLock  = 281;
        static constexpr KeyCode NumLock     = 282;
        static constexpr KeyCode PrintScreen = 283;
        static constexpr KeyCode Pause       = 284;

        static constexpr KeyCode F1  = 290;
        static constexpr KeyCode F2  = 291;
        static constexpr KeyCode F3  = 292;
        static constexpr KeyCode F4  = 293;
        static constexpr KeyCode F5  = 294;
        static constexpr KeyCode F6  = 295;
        static constexpr KeyCode F7  = 296;
        static constexpr KeyCode F8  = 297;
        static constexpr KeyCode F9  = 298;
        static constexpr KeyCode F10 = 299;
        static constexpr KeyCode F11 = 300;
        static constexpr KeyCode F12 = 301;

        static constexpr KeyCode LeftShift   = 340;
        static constexpr KeyCode LeftControl = 341;
        static constexpr KeyCode LeftAlt     = 342;
        static constexpr KeyCode LeftSuper   = 343;

        static constexpr KeyCode RightShift   = 344;
        static constexpr KeyCode RightControl = 345;
        static constexpr KeyCode RightAlt     = 346;
        static constexpr KeyCode RightSuper   = 347;

        static constexpr KeyCode Menu = 348;
    }
}
