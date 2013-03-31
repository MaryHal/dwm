#include "push.c"
#include <X11/XF86keysym.h>

/* appearance */
//static const char font[]          = "-*-terminus-medium-r-*-*-12-*-*-*-*-*-*-*";
//static const char font[]          = "Inconsolata:Semibold:size=9:antialias=true:hinting=true";
static const char font[]            =
    "Inconsolata:bold:size=9";
static const char normbordercolor[] = "#444444";
static const char normbgcolor[]     = "#222222";
static const char normfgcolor[]     = "#bbbbbb";
static const char selbordercolor[]  = "#005577";
static const char selbgcolor[]      = "#005577";
static const char selfgcolor[]      = "#eeeeee";
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 16;       /* snap pixel */
static const Bool showbar           = True;     /* False means no bar */
static const Bool topbar            = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
    /* class      instance    title       tags mask     isfloating   monitor */
    { "Gimp",     NULL,       NULL,       0,            True,        -1 },
    { "URxvt",    NULL,      "ncmpcpp",   0,            True,        -1 },
    { "Skype",    NULL,       NULL,       0,            True,        -1 },
//  { "Firefox",  NULL,       NULL,       1 << 8,       False,       -1 },
};

/* layout(s) */
static const float mfact       = 0.50;  /* factor of master area size [0.05..0.95] */
static const int   nmaster     = 1;     /* number of clients in master area */
static const Bool  resizehints = False; /* True means respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol           arrange function */
    { .symbol = "[]=", .arrange = tile },    /* first entry is default */
//  { .symbol = "><>", .arrange = NULL },    /* no layout function means floating behavior */
    { .symbol = "[M]", .arrange = monocle },
    { .symbol = NULL , .arrange = NULL },
};

/* key definitions */
#define ALTKEY Mod1Mask
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* Custom Functions */
static Monitor* dirToMon_NoCycle(int dir);    // This should probably be a patch...
static void focusMon_NoCycle(const Arg* arg);
static void tagMon_NoCycle(const Arg* arg);

static void nextLayout(const Arg* arg);
static void prevLayout(const Arg* arg);

/* commands */
static const char terminal[] = "urxvtc";
static const char *termcmd[]  = { terminal, NULL };
static const char *dmenucmd[] = { "dmenu_run", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, "-p", ">",  NULL };

static const char *alsamixer[]  = { terminal, "-e", "alsamixer", NULL };
static const char *ncmpcpp[]    = { terminal, "-geometry", "80x40", "-e", "ncmpcpp", NULL };

static const char *volumeUp[]   = { "amixer", "-q", "sset", "Master", "1+", "unmute", NULL };
static const char *volumeDown[] = { "amixer", "-q", "sset", "Master", "1-", "unmute", NULL };
static const char *volumeMute[] = { "amixer", "-q", "sset", "Master", "toggle", NULL };

static const char *mpcPrev[]    = { "mpc", "prev", "-q", NULL };
static const char *mpcNext[]    = { "mpc", "next", "-q", NULL };
static const char *mpcPlay[]    = { "mpc", "toggle", "-q", NULL };
static const char *mpcStop[]    = { "mpc", "stop", "-q", NULL };

static const char *mouseMove[]  = { "swarp", "0", "1080", NULL };

#define SCRIPT_DIR "/home/sanford/bin/"

static Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
    { MODKEY | ShiftMask,           XK_Return, spawn,          {.v = termcmd } },

    { MODKEY,                       XK_v,      spawn,          {.v = alsamixer } },
    { MODKEY,                       XK_n,      spawn,          {.v = ncmpcpp } },
    { MODKEY,                       XK_q,      spawn,          {.v = mouseMove } },
    { MODKEY,                       XK_r,      spawn,          SHCMD("/home/sanford/bin/randwall") },
    { MODKEY,                       XK_s,      spawn,          SHCMD("/home/sanford/bin/menu/shutdownmenu") },
    { MODKEY,                       XK_i,      spawn,          SHCMD("/home/sanford/bin/menu/brightnessmenu") },
    { MODKEY,                       XK_o,      spawn,          SHCMD("/home/sanford/bin/menu/mpcmenu") },
    { MODKEY,                       XK_u,      spawn,          SHCMD("/home/sanford/bin/menu/infomenu") },
    { MODKEY,                       XK_m,      spawn,          SHCMD("/home/sanford/bin/menu2") },

    { 0,         XF86XK_AudioLowerVolume,      spawn,          {.v = volumeDown } },
    { 0,         XF86XK_AudioRaiseVolume,      spawn,          {.v = volumeUp } },
    { 0,                XF86XK_AudioMute,      spawn,          {.v = volumeMute } },

    { 0,                XF86XK_AudioPrev,      spawn,          {.v = mpcPrev } },
    { 0,                XF86XK_AudioNext,      spawn,          {.v = mpcNext} },
    { 0,                XF86XK_AudioPlay,      spawn,          {.v = mpcPlay} },
    { 0,                XF86XK_AudioStop,      spawn,          {.v = mpcStop} },

    { MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
    { MODKEY,                       XK_Tab,    focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
    { MODKEY | ShiftMask,           XK_Tab,    focusstack,     {.i = -1 } },
    { MODKEY | ShiftMask,           XK_j,      pushdown,       {0} },
    { MODKEY | ShiftMask,           XK_k,      pushup,         {0} },
    { MODKEY,                       XK_comma,  incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_period, incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_h,      setmfact,       {.f = -0.01} },
    { MODKEY,                       XK_l,      setmfact,       {.f = +0.01} },
    { MODKEY,                       XK_g,      setmfact,       {.f = 0.5 } },
    { MODKEY,                       XK_Return, zoom,           {0} },
//  { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY | ShiftMask,           XK_c,      killclient,     {0} },
//  { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
//  { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
//  { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_t,      togglefloating, {0} },
    { MODKEY,                       XK_space,  nextLayout,     {0} },
    { MODKEY | ShiftMask,           XK_space,  setmfact,       {.f = 1.5f} },
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
    { MODKEY | ShiftMask,           XK_0,      tag,            {.ui = ~0 } },
    { MODKEY,                       XK_w,      focusMon_NoCycle,       {.i = -1 } },
    { MODKEY,                       XK_e,      focusMon_NoCycle,       {.i = +1 } },
    { MODKEY | ShiftMask,           XK_w,      tagMon_NoCycle,         {.i = -1 } },
    { MODKEY | ShiftMask,           XK_e,      tagMon_NoCycle,         {.i = +1 } },

    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)
//  { MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkWinTitle,          0,              Button2,        zoom,           {0} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

Monitor* dirToMon_NoCycle(int dir)
{
    if(dir > 0)
    {
        if(!selmon->next)
            return selmon;
        else
            return selmon->next;
    }
    else if (dir < 0)
    {
        if (selmon == mons) // If we're already at the first monitor
            return mons;
        else
        {
            Monitor* m = mons;
            for(; m->next != selmon; m = m->next);
            return m;
        }
    }
}

void focusMon_NoCycle(const Arg* arg)
{
    Monitor *m;

    if(!mons->next)
        return;
    if((m = dirToMon_NoCycle(arg->i)) == selmon)
        return;
    unfocus(selmon->sel, True);
    selmon = m;
    focus(NULL);
}

void tagMon_NoCycle(const Arg* arg)
{
    if(!selmon->sel || !mons->next)
        return;
    sendmon(selmon->sel, dirToMon_NoCycle(arg->i));
}

void nextLayout(const Arg* arg)
{
    Layout *l;
    for (l=(Layout *)layouts;l != selmon->lt[selmon->sellt];l++);
    if (l->symbol && (l + 1)->symbol)
        setlayout(&((Arg) { .v = (l + 1) }));
    else
        setlayout(&((Arg) { .v = layouts }));
}

void prevLayout(const Arg* arg)
{
    Layout *l;
    for (l=(Layout *)layouts;l != selmon->lt[selmon->sellt];l++);
    if (l != layouts && (l - 1)->symbol)
        setlayout(&((Arg) { .v = (l - 1) }));
    else
        setlayout(&((Arg) { .v = &layouts[LENGTH(layouts) - 2] }));
}
