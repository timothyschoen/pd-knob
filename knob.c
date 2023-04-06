// based on a proposal for vanilla

#include "m_pd.h"
#include "g_canvas.h"
#include <math.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define KNOB_SENSITIVITY 128
#define DEF_SIZE         35
#define MIN_SIZE         15

typedef struct _knob{
    t_object    x_obj;
    t_glist    *x_glist;
    int         x_size;
    t_float     x_pos; // 0-1 normalized position
    t_float     x_exp;
    int         x_expmode;
    t_float     x_init;
    int         x_start_angle;
    int         x_end_angle;
    int         x_range;
    int         x_offset;
    int         x_ticks;
    double      x_min;
    double      x_max;
    int         x_sel;
    int         x_shift;
    t_float     x_fval;
    t_symbol   *x_fg;
    t_symbol   *x_bg;
    t_symbol   *x_snd;
    t_symbol   *x_rcv;
    int         x_circular;
    int         x_arc;
    int         x_zoom;
    int         x_discrete;
    char        x_tag[128];         // generic tag
    char        x_tag_obj[128];
    char        x_tag_base[128];
    char        x_tag_sel[128];
    char        x_tag_arc[128];
    char        x_tag_wiper[128];
    char        x_tag_ticks[128];
    char        x_tag_center[128];
    char        x_tag_outline[128];
    char        x_tag_in[128];
    char        x_tag_out[128];
}t_knob;

t_widgetbehavior knob_widgetbehavior;
static t_class *knob_class;

static void knob_displace(t_gobj *z, t_glist *glist, int dx, int dy){
    t_knob *x = (t_knob *)z;
    x->x_obj.te_xpix += dx, x->x_obj.te_ypix += dy;
    dx *= x->x_zoom, dy *= x->x_zoom;
    pdgui_vmess(0, "crs ii", glist_getcanvas(glist), "move", x->x_tag_obj, dx, dy);
    canvas_fixlinesfor(glist, (t_text*)x);
}

static void knob_select(t_gobj *z, t_glist *glist, int sel){
    t_knob* x = (t_knob*)z;
    pdgui_vmess(0, "crs rs", glist_getcanvas(glist), "itemconfigure", x->x_tag_sel,
        "-outline", (x->x_sel = sel) ? "blue" : "black");
}

// get value from motion/position
static t_float knob_getfval(t_knob *x){
    t_float fval;
    t_float pos = x->x_pos;
    if(x->x_discrete){
        t_float ticks = (x->x_ticks < 2 ? 2 : (float)x->x_ticks) - 1;
        pos = rint(pos * ticks) / ticks;
    }
    if(x->x_expmode == 1){ // log
        if((x->x_min <= 0 && x->x_max >= 0) || (x->x_min >= 0 && x->x_max <= 0)){
            pd_error(x, "[knob]: range can't contain '0' in log mode");
            fval = x->x_min;
        }
        else
            fval = exp(pos * log(x->x_max / x->x_min)) * x->x_min;
    }
    else{
        if(x->x_expmode == 2){
            if(x->x_exp > 0)
                pos = pow(pos, x->x_exp);
            else
                pos = 1 - pow(1 - pos, -x->x_exp);
        }
        fval = pos * (x->x_max - x->x_min) + x->x_min;
    }
    if((fval < 1.0e-10) && (fval > -1.0e-10))
        fval = 0.0;
    return(fval);
}

// get position from value
static t_float knob_getpos(t_knob *x, t_floatarg fval){
    double pos;
    if(x->x_expmode == 1){ // log
        if((x->x_min <= 0 && x->x_max >= 0) || (x->x_min >= 0 && x->x_max <= 0)){
            pd_error(x, "[knob]: range cannot contain '0' in log mode");
            pos = 0;
        }
        else
            pos = log(fval/x->x_min) / log(x->x_max/x->x_min);
    }
    else{
        pos = (fval - x->x_min) / (x->x_max - x->x_min);
        if(x->x_expmode == 2){
            if(x->x_exp > 0)
                pos = pow(pos, 1.0/x->x_exp);
            else
                pos = 1-pow(1-pos, 1.0/(-x->x_exp));
        }
    }
    return(pos);
}

static void knob_draw_io(t_knob *x,t_glist *glist){
    int xpos = text_xpix(&x->x_obj, glist);
    int ypos = text_ypix(&x->x_obj, glist);
    int iow = IOWIDTH * x->x_zoom, ioh = OHEIGHT * x->x_zoom;
    t_canvas *cv = glist_getcanvas(glist);
    char *tags1[] = {x->x_tag_outline, x->x_tag_obj, x->x_tag_sel};
    pdgui_vmess(0, "crs", cv, "delete", x->x_tag_outline); // delete outline
    if(x->x_snd == gensym("empty") || x->x_rcv == gensym("empty")){
        pdgui_vmess(0, "crr iiii ri rS", cv, "create", "rectangle",
            xpos, ypos, xpos + x->x_size, ypos + x->x_size,
            "-width", x->x_zoom,
            "-tags", 3, tags1);
    }
    char *tags2[] = {x->x_tag_in, x->x_tag_obj};
    pdgui_vmess(0, "crs", cv, "delete", x->x_tag_in);
    if(x->x_rcv == gensym("empty")){
        pdgui_vmess(0, "crr iiii rs rS", cv, "create", "rectangle",
            xpos, ypos, xpos + iow, ypos - x->x_zoom + ioh,
            "-fill", "black",
            "-tags", 2, tags2);
    }
    char *tags3[] = {x->x_tag_out, x->x_tag_obj};
    pdgui_vmess(0, "crs", cv, "delete", x->x_tag_out);
    if(x->x_snd == gensym("empty")){
        pdgui_vmess(0, "crr iiii rs rS", cv, "create", "rectangle",
            xpos, ypos + x->x_size + x->x_zoom - ioh,
            xpos + iow, ypos + x->x_size,
            "-fill", "black",
            "-tags", 2, tags3);
    }
}

static void knob_update_ticks(t_knob *x, t_glist *glist){
    t_canvas *cv = glist_getcanvas(glist);
    pdgui_vmess(0, "crs", cv, "delete", x->x_tag_ticks);
    if(!x->x_ticks)
        return;
    int x0 = text_xpix(&x->x_obj, glist);
    int y0 = text_ypix(&x->x_obj, glist);
    float xc = x0 + x->x_size / 2.0;
    float yc = y0 + x->x_size / 2.0;
    float r1 = x->x_size / 2.0 - x->x_zoom * 2.0;
    float r2 = (float)x->x_zoom;
    int divs = x->x_ticks;
    if((divs > 1) && ((x->x_end_angle - x->x_start_angle + 360) % 360 != 0))
        divs = divs - 1;
    float dalpha = (x->x_end_angle - x->x_start_angle) / (float)divs;
    float alpha0 = x->x_start_angle;
    for(int tick = 0; tick < x->x_ticks; tick++){
        float alpha = (alpha0 + dalpha * tick - 90.0) * M_PI / 180.0;
        float xTc = xc + r1 * cos(alpha);
        float yTc = yc + r1 * sin(alpha);
        int x1 = rint(xTc - r2);
        int y1 = rint(yTc - r2);
        int x2 = rint(xTc + r2);
        int y2 = rint(yTc + r2);
        char *tags[] = {x->x_tag_ticks, x->x_tag_obj};
        int tickwidth = x->x_size / 20;
        if(tickwidth < 2)
            tickwidth = 2;
        pdgui_vmess(0, "crr iiii ri rsrs rS",
            cv, "create", "oval",
            x1, y1, x2, y2,
            "-width", tickwidth,
            "-outline", x->x_fg->s_name, "-fill", x->x_fg->s_name,
            "-tags", 2, tags);
    }
}

// Update wiper/arc according to [knob]'s value
static void knob_update_knob(t_knob *x, t_glist *glist){
    t_canvas *cv = glist_getcanvas(glist);
    float angle, angle0;
    t_float pos = x->x_pos;
    if(x->x_discrete){
        t_float ticks = (x->x_ticks < 2 ? 2 : (float)x->x_ticks) -1 ;
        pos = rint(pos * ticks) / ticks;
    }
    int x0 = text_xpix(&x->x_obj, glist);
    int y0 = text_ypix(&x->x_obj, glist);
    int x1 = x0 + x->x_size;
    int y1 = y0 + x->x_size;
    angle0 = (x->x_start_angle / 90.0 - 1) * M_PI / 2.0;
    angle = angle0 + pos * (x->x_end_angle - x->x_start_angle) / 180.0 * M_PI;
    if(x->x_arc){
        int arcwidth = x->x_arc * rint(x->x_size / 10);
        int aD, cD;
        angle0 += (knob_getpos(x, x->x_init) * x->x_range / 180.0 * M_PI);
        aD = x->x_zoom;
        // update arc
        pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_arc,
            x0 + aD, y0 + aD, x1 - aD, y1 - aD);
        pdgui_vmess(0, "crs sf sf", cv, "itemconfigure", x->x_tag_arc,
            "-start", angle0 * -180.0 / M_PI,
            "-extent", (angle - angle0) * -179.99 / M_PI);
        // update center
        cD = (arcwidth + 1) * x->x_zoom;
        pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_center,
            x0 + cD, y0 + cD, x1 - cD, y1 - cD);
    }
// wiper
    float radius = x->x_size / 2.0;
    int xc = rint((x0 + x1) / 2.0);
    int yc = rint((y0 + y1) / 2.0);
    int xp = xc + rint(radius * cos(angle));
    int yp = yc + rint(radius * sin(angle));
    pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_wiper, xc, yc, xp, yp);
}

static void knob_draw_update(t_knob *x, t_glist *glist){
    if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist))
        knob_update_knob(x, glist);
}

static void knob_config_fg(t_knob *x, t_canvas *cv){
    pdgui_vmess(0, "crs rsrs", cv, "itemconfigure",  x->x_tag_arc,
        "-outline", x->x_fg->s_name, "-fill", x->x_fg->s_name);
    pdgui_vmess(0, "crs rsrs", cv, "itemconfigure",  x->x_tag_ticks,
        "-outline", x->x_fg->s_name, "-fill", x->x_fg->s_name);
    pdgui_vmess(0, "crs rs", cv, "itemconfigure", x->x_tag_wiper,
        "-fill", x->x_fg->s_name);
}

static void knob_config_bg(t_knob *x, t_canvas *cv){
    pdgui_vmess(0, "crs rs", cv, "itemconfigure", x->x_tag_base,
        "-fill", x->x_bg->s_name);
    pdgui_vmess(0, "crs rsrs", cv, "itemconfigure", x->x_tag_center,
        "-outline", x->x_bg->s_name, "-fill", x->x_bg->s_name);
}

static void knob_config_colors(t_knob *x, t_canvas *cv){
    knob_config_bg(x, cv);
    knob_config_fg(x, cv);
}

static void knob_bgcolor(t_knob *x, t_symbol *s, int ac, t_atom *av){
    s = NULL;
    x->x_bg = atom_getsymbolarg(0, ac, av);
    knob_config_bg(x, glist_getcanvas(x->x_glist));
}

static void knob_fgcolor(t_knob *x, t_symbol *s, int ac, t_atom *av){
    s = NULL;
    x->x_fg = atom_getsymbolarg(0, ac, av);
    knob_config_fg(x, glist_getcanvas(x->x_glist));
}

// configure drawing elements
static void knob_draw_config(t_knob *x, t_glist *glist){
    t_canvas *cv = glist_getcanvas(glist);
// configure arc
    pdgui_vmess(0, "crs rs ri", cv, "itemconfigure", x->x_tag_arc,
        "-state", x->x_arc ? "normal" : "hidden",
        "-width", x->x_zoom); // really ??????
// configure wiper's width (why not size???)
    int wiperwidth = (x->x_size / 20);
    if(wiperwidth < 1)
        wiperwidth = 1;
    pdgui_vmess(0, "crs ri", cv, "itemconfigure", x->x_tag_wiper,
        "-width", wiperwidth * x->x_zoom); // do this with 'size'!?!!??!????
// knob circle (BASE) -> set size (why here???)
    int xpos = text_xpix(&x->x_obj, glist), ypos = text_ypix(&x->x_obj, glist);
    pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_base,
        xpos, ypos, xpos + x->x_size, ypos + x->x_size);
// more actions
    knob_config_colors(x, cv);
    knob_update_knob(x, glist);
    knob_update_ticks(x, glist);
}

static void knob_draw_new(t_knob *x, t_glist *glist){
// create drawing elements
    t_canvas *cv = glist_getcanvas(glist);
    char *tags1[] = {x->x_tag_base, x->x_tag_obj, x->x_tag_sel};
    pdgui_vmess(0, "crr iiii rS", cv, "create", "oval",
        0, 0, 0, 0,
        "-tags", 3, tags1);
    char *tags2[] = {x->x_tag_arc, x->x_tag_obj};
    pdgui_vmess(0, "crr iiii rS", cv, "create", "arc",
        0, 0, 0, 0,
        "-tags", 2, tags2);
    char *tags3[] = {x->x_tag_center, x->x_tag_obj};
    pdgui_vmess(0, "crr iiii rS", cv, "create", "oval",
        0, 0, 0, 0,
        "-tags", 2, tags3);
    char *tags4[] = {x->x_tag_wiper, x->x_tag_obj};
    pdgui_vmess(0, "crr iiii rS", cv, "create", "line",
        0, 0, 0, 0,
        "-tags", 2, tags4);
    knob_draw_io(x, glist); // knob outline, inlet/outlet
    knob_draw_config(x, glist);
    pdgui_vmess(0, "crs rs", glist_getcanvas(glist), "itemconfigure", x->x_tag_sel,
        "-outline", x->x_sel ? "blue" : "black");
}

static void knob_send(t_knob *x, t_symbol *s){
    x->x_snd = s;
}

static void knob_receive(t_knob *x, t_symbol *s){
    x->x_rcv = s;
}

void knob_vis(t_gobj *z, t_glist *glist, int vis){
    t_knob* x = (t_knob*)z;
    if(vis)
        knob_draw_new(x, glist);
    else
        pdgui_vmess(0, "crs", glist_getcanvas(glist), "delete", x->x_tag_obj);
}

static void knob_delete(t_gobj *z, t_glist *glist){
    canvas_deletelinesfor(glist, (t_text *)z);
}

// ------------------------ knob widgetbehaviour-----------------------------
static void knob_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2){
    t_knob *x = (t_knob *)z;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + x->x_size;
    *yp2 = text_ypix(&x->x_obj, glist) + x->x_size;
}

static void knob_save(t_gobj *z, t_binbuf *b){
    t_knob *x = (t_knob *)z;
    binbuf_addv(b, "ssiis",
        gensym("#X"),
        gensym("obj"),
        (t_int)x->x_obj.te_xpix,
        (t_int)x->x_obj.te_ypix,
        atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)));
    binbuf_addv(b, "ifffissssfiiiiii", // 15 args
        x->x_size / x->x_zoom, // 01: i SIZE
        (float)x->x_min, // 02: f min
        (float)x->x_max, // 03: f max
        x->x_exp, // 04: f exp
        x->x_expmode, // 05: i expmode
        x->x_rcv, // 06: s rcv
        x->x_snd, // 07: s snd
        x->x_bg, // 08: s bgcolor
        x->x_fg, // 09: s fgcolor
        x->x_init, // 10: f init
        x->x_circular, // 11: i circular
        x->x_ticks, // 12: i ticks
        x->x_discrete, // 13: i discrete
        x->x_arc, // 14: i arc
        x->x_range, // 15: i range
        x->x_offset); // 16: i offset
    binbuf_addv(b, ";");
}

static void knob_range(t_knob *x, t_floatarg f1, t_floatarg f2){
    if(f1 > f2){
        x->x_max = (double)f1;
        x->x_min = (double)f2;
    }
    else{
        x->x_min = (double)f1;
        x->x_max = (double)f2;
    }
}

static void knob_exp(t_knob *x, t_floatarg f){
    if(f == 0 || f == 1)
        x->x_expmode = f;
    else{
        x->x_expmode = 2;
        x->x_exp = f;
    }
}

static void knob_discrete(t_knob *x, t_floatarg f){
    x->x_discrete = (f != 0);
}

static void knob_properties(t_gobj *z, t_glist *owner){
    owner = NULL;
    t_knob *x = (t_knob *)z;
    char buffer[512];
    sprintf(buffer, "knob_dialog %%s %g %g %g %g %g %d {%s} {%s} %d %g {%s} {%s} %d %d %d %d %d \n",
        (float)(x->x_size / x->x_zoom),
        (float)MIN_SIZE,
        x->x_min,
        x->x_max,
        x->x_init,
        x->x_circular,
        x->x_snd->s_name,
        x->x_rcv->s_name,
        x->x_expmode,
        x->x_exp,
        x->x_bg->s_name,
        x->x_fg->s_name,
        x->x_discrete,
        x->x_ticks,
        x->x_arc,
        x->x_range,
        x->x_offset);
    gfxstub_new(&x->x_obj.ob_pd, x, buffer);
}
    
static void knob_set(t_knob *x, t_floatarg f){
    float old = x->x_pos;
    x->x_fval = f > x->x_max ? x->x_max : f < x->x_min ? x->x_min : f;
    x->x_pos = knob_getpos(x, x->x_fval);
    if(x->x_pos != old)
        knob_draw_update(x, x->x_glist);
}

static void knob_bang(t_knob *x){
    outlet_float(x->x_obj.ob_outlet, x->x_fval);
    if(x->x_snd->s_thing)
        pd_float(x->x_snd->s_thing, x->x_fval);
}

static void knob_float(t_knob *x, t_floatarg f){
    knob_set(x, f);
    knob_bang(x);
}

static void knob_init(t_knob *x, t_symbol *s, int ac, t_atom *av){
    s = NULL;
    if(ac == 1 && av->a_type == A_FLOAT)
        knob_float(x, atom_getfloat(av));
    x->x_init = x->x_fval;
}

// set start/end angles
static void knob_angle(t_knob *x, t_floatarg f1, t_floatarg f2){
    x->x_range = f1 > 360 ? 360 : f1 < 0 ? 0 : (int)f1;
    x->x_offset = f2 > 360 ? 360 : f2 < 0 ? 0 : (int)f2;
    int start = -(x->x_range/2) + x->x_offset;
    int end = x->x_range/2 + x->x_offset;
    float tmp;
    while(start < -360)
        start = -360;
    while (start > 360)
        start = 360;
    while(end < -360)
        end = -360;
    while(end > 360)
        end = 360;
    if(end < start){
        tmp = start;
        start = end;
        end = tmp;
    }
    if((end - start) > 360)
        end = start + 360;
    if(end == start)
        end = start + 1;
    x->x_start_angle = start;
    x->x_end_angle = end;
    knob_set(x, x->x_fval);
    if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist))
        knob_update_ticks(x, x->x_glist);
    knob_draw_update(x, x->x_glist);
}

static void knob_size(t_knob *x, t_floatarg f){
    float size = f < MIN_SIZE ? MIN_SIZE : f;
    if(x->x_size != size){
        x->x_size = size;
        if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist)){
            knob_draw_config(x, x->x_glist);
            knob_draw_update(x, x->x_glist);
            int outline = 1;
            if(outline){
                int xpos = text_xpix(&x->x_obj, x->x_glist);
                int ypos = text_ypix(&x->x_obj, x->x_glist);
                t_canvas *cv = glist_getcanvas(x->x_glist);
                pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_outline,
                    xpos, ypos, xpos + x->x_size, ypos + x->x_size);
                pdgui_vmess(0, "crs iiii", cv, "coords", x->x_tag_out,
                    xpos, ypos + x->x_size * x->x_zoom - OHEIGHT * x->x_zoom,
                    xpos + IOWIDTH * x->x_zoom, ypos + x->x_size * x->x_zoom);
            }
            canvas_fixlinesfor(x->x_glist, (t_text *)x);
        }
    }
}

static void knob_apply(t_knob *x, t_symbol *s, int ac, t_atom *av){
    s = NULL;
    int size = (int)atom_getintarg(0, ac, av);
    float min = atom_getfloatarg(1, ac, av);
    float max = atom_getfloatarg(2, ac, av);
    double init = atom_getfloatarg(3, ac, av);
    t_symbol* snd = atom_getsymbolarg(4, ac, av);
    t_symbol* rcv = atom_getsymbolarg(5, ac, av);
    x->x_expmode = atom_getintarg(6, ac, av);
    x->x_exp = atom_getfloatarg(7, ac, av);
    x->x_bg = atom_getsymbolarg(8, ac, av);
    x->x_fg = atom_getsymbolarg(9, ac, av);
    int circular = atom_getintarg(10, ac, av);
    int ticks = atom_getintarg(11, ac, av);
    int discrete = atom_getintarg(12, ac, av);
    int arc = atom_getintarg(13, ac, av) != 0;
    int range = atom_getintarg(14, ac, av);
    int offset = atom_getintarg(15, ac, av);
    
    t_atom undo[16];
    SETFLOAT(undo+0, x->x_size);
    SETFLOAT(undo+1, x->x_min);
    SETFLOAT(undo+2, x->x_max);
    SETFLOAT(undo+3, x->x_init);
    SETSYMBOL(undo+4, x->x_snd);
    SETSYMBOL(undo+5, x->x_rcv);
    SETFLOAT(undo+6, x->x_expmode);
    SETFLOAT(undo+7, x->x_exp);
    SETSYMBOL(undo+8, x->x_bg);
    SETSYMBOL(undo+9, x->x_fg);
    SETFLOAT(undo+10, x->x_circular);
    SETFLOAT(undo+11, x->x_ticks);
    SETFLOAT(undo+12, x->x_discrete);
    SETFLOAT(undo+13, x->x_arc);
    SETFLOAT(undo+14, x->x_range);
    SETFLOAT(undo+15, x->x_offset);
    pd_undo_set_objectstate(x->x_glist, (t_pd*)x, gensym("dialog"),
        16, undo, ac, av);
    if(x->x_init != init){
        knob_float(x, init);
        x->x_init = x->x_fval;
    }
    post("x->x_bg (%s), x->x_fg (%s)", x->x_bg->s_name, x->x_fg->s_name);
    knob_config_bg(x, glist_getcanvas(x->x_glist));
    knob_config_fg(x, glist_getcanvas(x->x_glist));
    x->x_circular = circular;
    x->x_ticks = ticks < 0 ? 0 : ticks;
    x->x_arc = arc;
    x->x_discrete = discrete;
    knob_angle(x, (float)range, (float)offset);
    knob_size(x, size);
    knob_discrete(x, discrete);
    knob_range(x, min, max);
    knob_set(x, x->x_fval);
    knob_send(x, snd);
    knob_receive(x, rcv);
}

static void knob_motion(t_knob *x, t_floatarg dx, t_floatarg dy){
    float old = x->x_pos;
    float delta = -dy;
    if(fabs(dx) > fabs(dy))
        delta = dx;
    delta /= (float)(KNOB_SENSITIVITY * x->x_zoom);
    if(x->x_shift)
        delta *= 0.01;
    double pos = x->x_pos + delta;
    x->x_pos = pos > 1 ? 1 : pos < 0 ? 0 : pos;
    x->x_fval = knob_getfval(x);
    if(old != x->x_pos){
        knob_draw_update(x, x->x_glist);
        knob_bang(x);
    }
}

static int xm, ym;

static void knob_motion_angular(t_knob *x, t_floatarg dx, t_floatarg dy){
    int xc = text_xpix(&x->x_obj, x->x_glist) + x->x_size / 2;
    int yc = text_ypix(&x->x_obj, x->x_glist) + x->x_size / 2;
    float old = x->x_pos;
    float alpha;
    float alphacenter = (x->x_end_angle + x->x_start_angle) / 2;
    xm += dx;
    ym += dy;
    alpha = atan2(xm - xc, -ym + yc) * 180.0 / M_PI;
    x->x_pos = (((int)((alpha - alphacenter + 180.0 + 360.0) * 100.0) % 36000) * 0.01
        + (alphacenter - x->x_start_angle - 180.0)) / (x->x_end_angle - x->x_start_angle);
    if(x->x_pos < 0)
        x->x_pos = 0;
    if(x->x_pos > 1.0)
        x->x_pos = 1.0;
    x->x_fval = knob_getfval(x);
    if(old != x->x_pos){
        knob_draw_update(x, x->x_glist);
        knob_bang(x);
    }
}

static void knob_click(t_knob *x, t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl, t_floatarg alt){
    alt = ctrl = shift = 0;
    knob_bang(x);
    glist_grab(x->x_glist, &x->x_obj.te_g,
        (t_glistmotionfn)(x->x_circular ? knob_motion_angular : knob_motion), 0, xm = xpos, ym = ypos);
}

static int knob_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit){
    glist = 0;
    t_knob *x = (t_knob *)z;
    if(dbl){
        knob_set(x, x->x_init);
        knob_bang(x);
        return(1);
    }
    if(doit){
        x->x_shift = shift;
        knob_click(x, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift, 0, (t_floatarg)alt);
    }
    return(1);
}

static void knob_circular(t_knob *x, t_floatarg f){
    x->x_circular = (f != 0);
}

static void knob_arc(t_knob *x, t_floatarg f){
    x->x_arc = f != 0;
    if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist)){
        knob_draw_config(x, x->x_glist);
        knob_draw_update(x, x->x_glist);
    }
}

static void knob_ticks(t_knob *x, t_floatarg f){
    x->x_ticks = (int)f;
    if(f <= 0)
        x->x_ticks = 0;
    if(f == 1)
        x->x_ticks = 2;
    if(f > 100)
        x->x_ticks = 100;
    if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist))
        knob_update_ticks(x, x->x_glist);
}

static void knob_zoom(t_knob *x, t_floatarg f){
    x->x_zoom = (int)f;
}

static void *knob_new(t_symbol *s, int ac, t_atom *av){
    s = NULL;
    t_knob *x = (t_knob *)pd_new(knob_class);
    float initvalue = 0, exp = 0;
    x->x_snd = gensym("empty");
    x->x_rcv = gensym("empty");
    int size = 30, circular = 0, ticks = 0, discrete = 0, expmode = 0;
    int arc = 1, range = 360, offset = 0;
    double min = 0.0, max = 127.0;
    x->x_bg = gensym("#dfdfdf"), x->x_fg = gensym("black");
    x->x_glist = (t_glist *)canvas_getcurrent();
    x->x_zoom = x->x_glist->gl_zoom;
    if(ac){
        size = atom_getintarg(0, ac, av);
        min = (double)atom_getfloatarg(1, ac, av);
        max = (double)atom_getfloatarg(2, ac, av);
        exp = atom_getfloatarg(3, ac, av);
        expmode = atom_getfloatarg(4, ac, av);
        x->x_snd = atom_getsymbolarg(5, ac, av);
        x->x_rcv = atom_getsymbolarg(6, ac, av);
        x->x_bg = atom_getsymbolarg(7, ac, av);
        x->x_fg = atom_getsymbolarg(8, ac, av);
        initvalue = atom_getfloatarg(9, ac, av);
        circular = atom_getintarg(10, ac, av);
        ticks = atom_getintarg(11, ac, av);
        discrete = atom_getintarg(12, ac, av);
        arc = atom_getintarg(13, ac, av);
        range = atom_getintarg(14, ac, av);
        offset = atom_getintarg(15, ac, av);
    }
    x->x_expmode = expmode;
    x->x_exp = exp;
    x->x_circular = circular;
    x->x_glist = (t_glist *)canvas_getcurrent();
    knob_ticks(x, ticks);
    x->x_discrete = discrete;
    x->x_arc = arc;
    knob_angle(x, range, offset);
    knob_size(x, size);
    knob_range(x, min, max);
    knob_set(x, x->x_init = initvalue);
    sprintf(x->x_tag_obj, "%pOBJ", x);
    sprintf(x->x_tag_base, "%pBASE", x);
    sprintf(x->x_tag_sel, "%pSEL", x);
    sprintf(x->x_tag_arc, "%pARC", x);
    sprintf(x->x_tag_ticks, "%pTICKS", x);
    sprintf(x->x_tag_wiper, "%pWIPER", x);
    sprintf(x->x_tag_center, "%pCENTER", x);
    sprintf(x->x_tag_outline, "%pOUTLINE", x);
    sprintf(x->x_tag_in, "%pIN", x);
    sprintf(x->x_tag_out, "%pOUT", x);
    if(x->x_rcv != gensym("empty"))
        pd_bind(&x->x_obj.ob_pd, x->x_rcv);
    outlet_new(&x->x_obj, &s_float);
    return(x);
}

static void knob_free(t_knob *x){
    if(x->x_rcv != gensym("empty"))
        pd_unbind(&x->x_obj.ob_pd, x->x_rcv);
    gfxstub_deleteforkey(x);
}

void knob_setup(void){
    knob_class = class_new(gensym("knob"), (t_newmethod)knob_new,
        (t_method)knob_free, sizeof(t_knob), 0, A_GIMME, 0);
    class_addbang(knob_class,knob_bang);
    class_addfloat(knob_class,knob_float);
    class_addmethod(knob_class, (t_method)knob_init, gensym("init"), A_GIMME, 0);
    class_addmethod(knob_class, (t_method)knob_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_size, gensym("size"), A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_circular, gensym("circular"), A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_range, gensym("range"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_exp, gensym("exp"), A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_discrete, gensym("discrete"), A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_bgcolor, gensym("bgcolor"), A_GIMME, 0);
    class_addmethod(knob_class, (t_method)knob_fgcolor, gensym("fgcolor"), A_GIMME, 0);
    class_addmethod(knob_class, (t_method)knob_send, gensym("send"), A_DEFSYM, 0);
    class_addmethod(knob_class, (t_method)knob_receive, gensym("receive"), A_DEFSYM, 0);
    class_addmethod(knob_class, (t_method)knob_arc, gensym("arc"), A_DEFFLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_angle, gensym("angle"), A_FLOAT, A_DEFFLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_ticks, gensym("ticks"), A_DEFFLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_zoom, gensym("zoom"), A_CANT, 0);
    class_addmethod(knob_class, (t_method)knob_apply, gensym("dialog"), A_GIMME, 0);
    class_addmethod(knob_class, (t_method)knob_click, gensym("click"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(knob_class, (t_method)knob_motion, gensym("motion"), A_FLOAT, A_FLOAT, 0);
    knob_widgetbehavior.w_getrectfn  = knob_getrect;
    knob_widgetbehavior.w_displacefn = knob_displace;
    knob_widgetbehavior.w_selectfn   = knob_select;
    knob_widgetbehavior.w_activatefn = NULL;
    knob_widgetbehavior.w_deletefn   = knob_delete;
    knob_widgetbehavior.w_visfn      = knob_vis;
    knob_widgetbehavior.w_clickfn    = knob_newclick;
    class_setwidget(knob_class, &knob_widgetbehavior);
    class_setsavefn(knob_class, knob_save);
    class_setpropertiesfn(knob_class, knob_properties);
    #include "knob_dialog.h"
}
