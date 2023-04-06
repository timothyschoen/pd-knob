// Based on dialogs from iemguis in Pd Vanilla (Tim Schoen & Porres)

sys_gui("\n"
"\n"
"namespace eval ::dialog_knob:: {\n"
"}\n"
"\n"
"\n"
"# arrays to store per-dialog values\n"
"\n"
"array set ::dialog_knob::var_size {} ;\n"
"array set ::dialog_knob::var_minsize {} ;\n"
"\n"
"array set ::dialog_knob::var_range_max {} ;\n"
"array set ::dialog_knob::var_range_min {} ;\n"
"\n"
"array set ::dialog_knob::var_initial {} ;\n"
"array set ::dialog_knob::var_circular {} ;\n"
"\n"
"array set ::dialog_knob::var_ticks {} ;\n"
"array set ::dialog_knob::var_arc_shown {} ;\n"
"array set ::dialog_knob::var_angle_width {} ;\n"
"array set ::dialog_knob::var_angle_offset {} ;\n"
"array set ::dialog_knob::var_discrete {} ;\n"
"\n"
"array set ::dialog_knob::var_snd {} ;\n"
"array set ::dialog_knob::var_rcv {} ;\n"
"\n"
"array set ::dialog_knob::var_color_background {} ;\n"
"array set ::dialog_knob::var_color_foreground {} ;\n"
"array set ::dialog_knob::var_colortype {} ;\n"
"\n"
"proc ::dialog_knob::preset_col {mytoplevel presetcol} {\n"
"    set vid [string trimleft $mytoplevel .]\n"
"\n"
"    switch -- $::dialog_knob::var_colortype($vid) {\n"
"        0 { set ::dialog_knob::var_color_background($vid) $presetcol }\n"
"        1 { set ::dialog_knob::var_color_foreground($vid) $presetcol }\n"
"    }\n"
"    if {$::windowingsystem eq \"aqua\"} {\n"
"    ::dialog_knob::apply_and_rebind_return $mytoplevel \n"
"    }\n"
"}\n"
"\n"
"proc ::dialog_knob::choose_col_bkfrlb {mytoplevel} {\n"
"    # TODO rename this\n"
"    set vid [string trimleft $mytoplevel .]\n"
"\n"
"    switch -- $::dialog_knob::var_colortype($vid) {\n"
"        0 {\n"
"            set title [_ \"Background color\" ]\n"
"            set color $::dialog_knob::var_color_background($vid)\n"
"        }\n"
"        1 {\n"
"            set title [_ \"Foreground color\" ]\n"
"            set color $::dialog_knob::var_color_foreground($vid)\n"
"        }\n"
"    }\n"
"    set color [tk_chooseColor -title $title -initialcolor $color]\n"
"\n"
"    if { $color ne \"\" } {\n"
"        ::dialog_knob::preset_col $color\n"
"    }\n"
"}\n"
"\n"
"proc ::dialog_knob::clip {val min {max {}}} {\n"
"    if {$min ne {} && $val < $min} {return $min}\n"
"    if {$max ne {} && $val > $max} {return $max}\n"
"    return $val\n"
"}\n"
"\n"
"proc ::dialog_knob::apply {mytoplevel} {\n"
"    set vid [string trimleft $mytoplevel .]\n"
"\n"
"    # TODO wrap the name-mangling ('empty', unspace_text, map) into a helper-proc\n"
"    set sendname empty\n"
"    set receivename empty\n"
"    set labelname empty\n"
"\n"
"    if {$::dialog_knob::var_snd($vid) ne \"\"} {set sendname $::dialog_knob::var_snd($vid)}\n"
"    if {$::dialog_knob::var_rcv($vid) ne \"\"} {set receivename $::dialog_knob::var_rcv($vid)}\n"
"\n"
"    set ::dialog_knob::var_ticks($vid) [::dialog_knob::clip $::dialog_knob::var_ticks($vid) 0 360]\n"
"\n"
"    pdsend [concat $mytoplevel dialog \\\n"
"                $::dialog_knob::var_size($vid) \\\n"
"                $::dialog_knob::var_range_min($vid) \\\n"
"                $::dialog_knob::var_range_max($vid) \\\n"
"                $::dialog_knob::var_initial($vid) \\\n"
"                [string map {\"$\" {\\$}} [unspace_text $sendname]] \\\n"
"                [string map {\"$\" {\\$}} [unspace_text $receivename]] \\\n"
"                $::dialog_knob::var_expmode($vid) \\\n"
"                $::dialog_knob::var_exp($vid) \\\n"
"                [string tolower $::dialog_knob::var_color_background($vid)] \\\n"
"                [string tolower $::dialog_knob::var_color_foreground($vid)] \\\n"
"                $::dialog_knob::var_circular($vid) \\\n"
"                $::dialog_knob::var_ticks($vid) \\\n"
"                $::dialog_knob::var_discrete($vid) \\\n"
"                $::dialog_knob::var_arc_shown($vid) \\\n"
"                $::dialog_knob::var_angle_width($vid) \\\n"
"                $::dialog_knob::var_angle_offset($vid) \\\n"
"            ]\n"
"}\n"
"\n"
"\n"
"proc ::dialog_knob::cancel {mytoplevel} {\n"
"    pdsend \"$mytoplevel cancel\"\n"
"}\n"
"\n"
"proc ::dialog_knob::ok {mytoplevel} {\n"
"    ::dialog_knob::apply $mytoplevel\n"
"    ::dialog_knob::cancel $mytoplevel\n"
"}\n"
"\n"
"proc knob_dialog {mytoplevel \\\n"
"                                       wdt min_wdt \\\n"
"                                       min_rng max_rng \\\n"
"                                       initial circular \\\n"
"                                       snd rcv \\\n"
"                                       expmode exp \\\n"
"                                       bcol fcol \\\n"
"                                       discrete ticks \\\n"
"                                       arc_width start_angle end_angle} {\n"
"\n"
"    set vid [string trimleft $mytoplevel .]\n"
"    set snd [::pdtk_text::unescape $snd]\n"
"    set rcv [::pdtk_text::unescape $rcv]\n"
"\n"
"    # initialize the array\n"
"\n"
"    set ::dialog_knob::var_size($vid) $wdt\n"
"    set ::dialog_knob::var_minsize($vid) $min_wdt\n"
"\n"
"    set ::dialog_knob::var_range_max($vid) $max_rng\n"
"    set ::dialog_knob::var_range_min($vid) $min_rng\n"
"\n"
"    set ::dialog_knob::var_initial($vid) $initial\n"
"    set ::dialog_knob::var_circular($vid) $circular\n"
"    set ::dialog_knob::var_expmode($vid) $expmode\n"
"    set ::dialog_knob::var_exp($vid) $exp\n"
"\n"
"    set ::dialog_knob::var_snd($vid) $snd\n"
"    set ::dialog_knob::var_rcv($vid) $rcv\n"
"\n"
"    set ::dialog_knob::var_color_background($vid) $bcol\n"
"    set ::dialog_knob::var_color_foreground($vid) $fcol\n"
"    set ::dialog_knob::var_colortype($vid) 0\n"
"\n"
"    set ::dialog_knob::var_discrete($vid) $discrete\n"
"    set ::dialog_knob::var_ticks($vid) $ticks\n"
"    set ::dialog_knob::var_arc_shown($vid) $arc_width\n"
"    set ::dialog_knob::var_angle_width($vid) $start_angle\n"
"    set ::dialog_knob::var_angle_offset($vid) $end_angle\n"
"\n"
"    toplevel $mytoplevel -class DialogWindow\n"
"    wm title $mytoplevel \"knob properties\"\n"
"    wm group $mytoplevel .\n"
"    wm resizable $mytoplevel 0 0\n"
"    wm transient $mytoplevel $::focused_window\n"
"    $mytoplevel configure -menu $::dialog_menubar\n"
"    $mytoplevel configure -padx 0 -pady 0\n"
"    ::pd_bindings::dialog_bindings $mytoplevel \"iemgui\"\n"
"\n"
"    # parameters\n"
"    frame $mytoplevel.para -padx 5 -pady 5 \n"
"    pack $mytoplevel.para -side top -fill x -pady 5\n"
"\n"
"    set applycmd \"\"\n"
"    if {$::windowingsystem eq \"aqua\"} {\n"
"        set applycmd \"::dialog_knob::apply $mytoplevel\"\n"
"    }\n"
"\n"
"    # style\n"
"    frame $mytoplevel.para.knobstyle -padx 20 -pady 1\n"
"\n"
"    frame $mytoplevel.para.knobstyle.initial \n"
"    label $mytoplevel.para.knobstyle.initial.lab -text [_ \"Initial Value\"]\n"
"    entry $mytoplevel.para.knobstyle.initial.ent -textvariable ::dialog_knob::var_initial($vid) -width 4\n"
"    pack $mytoplevel.para.knobstyle.initial.lab -side left -expand 0 -ipadx 4\n"
"    pack $mytoplevel.para.knobstyle.initial.ent -side left -expand 0 -ipadx 10\n"
"    destroy $mytoplevel.para.knobstyle.stdy_jmp\n"
"    frame $mytoplevel.para.knobstyle.move \n"
"    label $mytoplevel.para.knobstyle.move.lab -text [_ \"Move Mode:\"]\n"
"    ::dialog_knob::popupmenu_strval $mytoplevel.para.knobstyle.move.mode \\\n"
"        ::dialog_knob::var_circular($vid) \\\n"
"        [list 0 1] [list [_ \"Normal\"] [_ \"Circular\"] ] \\\n"
"        $applycmd\n"
"    pack $mytoplevel.para.knobstyle.move.lab -side left -expand 0 -ipadx 4\n"
"    pack $mytoplevel.para.knobstyle.move.mode -side left -expand 0 -ipadx 10\n"
"\n"
"    frame $mytoplevel.para.knobstyle.ticks\n"
"    label $mytoplevel.para.knobstyle.ticks.lab -text [_ \"Ticks: \"]\n"
"    entry $mytoplevel.para.knobstyle.ticks.ent -textvariable ::dialog_knob::var_ticks($vid) -width 5\n"
"    pack $mytoplevel.para.knobstyle.ticks.ent $mytoplevel.para.knobstyle.ticks.lab -side right -anchor e\n"
"\n"
"    frame $mytoplevel.para.knobstyle.arc\n"
"    label $mytoplevel.para.knobstyle.arc.lab -text [_ \"Arc: \"]\n"
"    checkbutton $mytoplevel.para.knobstyle.arc.ent -variable ::dialog_knob::var_arc_shown($vid) -width 5\n"
"    pack $mytoplevel.para.knobstyle.arc.ent $mytoplevel.para.knobstyle.arc.lab -side right -anchor e\n"
"\n"
"    frame $mytoplevel.para.knobstyle.start\n"
"    label $mytoplevel.para.knobstyle.start.lab -text [_ \"Angular range: \"]\n"
"    entry $mytoplevel.para.knobstyle.start.ent -textvariable ::dialog_knob::var_angle_width($vid) -width 5\n"
"    pack $mytoplevel.para.knobstyle.start.ent $mytoplevel.para.knobstyle.start.lab -side right -anchor e\n"
"\n"
"    frame $mytoplevel.para.knobstyle.end\n"
"    label $mytoplevel.para.knobstyle.end.lab -text [_ \"Offset: \"]\n"
"    entry $mytoplevel.para.knobstyle.end.ent -textvariable ::dialog_knob::var_angle_offset($vid) -width 5\n"
"    pack $mytoplevel.para.knobstyle.end.ent $mytoplevel.para.knobstyle.end.lab -side right -anchor w\n"
"\n"
"    frame $mytoplevel.para.knobstyle.discrete\n"
"    label $mytoplevel.para.knobstyle.discrete.lab -text [_ \"Discrete: \"]\n"
"    checkbutton $mytoplevel.para.knobstyle.discrete.ent -variable ::dialog_knob::var_discrete($vid) -width 5\n"
"    pack $mytoplevel.para.knobstyle.discrete.ent $mytoplevel.para.knobstyle.discrete.lab -side right -anchor e\n"
"\n"
"    frame $mytoplevel.para.knobstyle.dim \n"
"    label $mytoplevel.para.knobstyle.dim.w_lab -text \"Size:\"\n"
"    entry $mytoplevel.para.knobstyle.dim.w_ent -textvariable ::dialog_knob::var_size($vid) -width 4\n"
"    label $mytoplevel.para.knobstyle.dim.dummy1 -text \"\" -width 1\n"
"    pack $mytoplevel.para.knobstyle.dim.w_lab $mytoplevel.para.knobstyle.dim.w_ent -side left\n"
"\n"
"    grid $mytoplevel.para.knobstyle.dim -row 0 -column 0 -sticky e -padx {5 0}\n"
"    grid $mytoplevel.para.knobstyle.initial -row 1 -column 0 -sticky e -padx {5 0}\n"
"    grid $mytoplevel.para.knobstyle.start -row 2 -column 0 -sticky e -padx {5 0}\n"
"    grid $mytoplevel.para.knobstyle.end -row 3 -column 0 -sticky e -padx {5 0}\n"
"\n"
"    grid $mytoplevel.para.knobstyle.ticks -row 0 -column 1 -sticky e -padx {5 0}\n"
"    grid $mytoplevel.para.knobstyle.move -row 1 -column 1 -sticky e -padx {5 0} \n"
"    grid $mytoplevel.para.knobstyle.arc -row 2 -column 1 -sticky e -padx {5 0}\n"
"    grid $mytoplevel.para.knobstyle.discrete -row 3 -column 1 -sticky e -padx {5 0}\n"
"\n"
"    pack $mytoplevel.para.knobstyle -side top -fill x\n"
"\n"
"    # range\n"
"    labelframe $mytoplevel.rng\n"
"    pack $mytoplevel.rng -side top -fill x\n"
"    frame $mytoplevel.rng.range \n"
"    frame $mytoplevel.rng.range.min\n"
"    label $mytoplevel.rng.range.min.lab -text \"Min:\"\n"
"    entry $mytoplevel.rng.range.min.ent -textvariable ::dialog_knob::var_range_min($vid) -width 7\n"
"    label $mytoplevel.rng.range.dummy1 -text \"\" -width 1\n"
"    label $mytoplevel.rng.range.max_lab -text \"Max:\"\n"
"    entry $mytoplevel.rng.range.max_ent -textvariable ::dialog_knob::var_range_max($vid) -width 7\n"
"    $mytoplevel.rng config -borderwidth 1 -pady 4 -text \"Range:\"\n"
"    pack $mytoplevel.rng.range.min\n"
"    pack $mytoplevel.rng.range.min.lab $mytoplevel.rng.range.min.ent -side left \n"
"    $mytoplevel.rng config -padx 26\n"
"    pack configure $mytoplevel.rng.range.min -side left\n"
"    pack $mytoplevel.rng.range.dummy1 $mytoplevel.rng.range.max_lab $mytoplevel.rng.range.max_ent -side left\n"

"\n"
"    frame $mytoplevel.rng.logmode\n"
"    radiobutton $mytoplevel.rng.logmode.radio1 -value 0 \\\n"
"        -text [_ \"linear\" ] \\\n"
"        -variable ::dialog_knob::var_expmode($vid) \\\n"
"        -command \"$mytoplevel.rng.logmode.expmode_entry configure -state disabled\"\n"
"\n"
"    radiobutton $mytoplevel.rng.logmode.radio2 -value 1 \\\n"
"        -text [_ \"log\" ] \\\n"
"        -variable ::dialog_knob::var_expmode($vid) \\\n"
"        -command \"$mytoplevel.rng.logmode.expmode_entry configure -state disabled\"\n"
"\n"
"    radiobutton $mytoplevel.rng.logmode.radio3 -value 2 \\\n"
"        -text [_ \"exp:\" ] \\\n"
"        -variable ::dialog_knob::var_expmode($vid) \\\n"
"        -command \"$mytoplevel.rng.logmode.expmode_entry configure -state normal\"\n"
"    entry $mytoplevel.rng.logmode.expmode_entry -width 3 -textvariable ::dialog_knob::var_exp($vid) \n"
"    if { $::dialog_knob::var_expmode($vid) != 2 } {\n"
"       $mytoplevel.rng.logmode.expmode_entry configure -state disabled\n"
"    }\n"
"    pack $mytoplevel.rng.logmode.expmode_entry $mytoplevel.rng.logmode.radio3 $mytoplevel.rng.logmode.radio2 $mytoplevel.rng.logmode.radio1 -side right \n"
"    pack $mytoplevel.rng.range $mytoplevel.rng.logmode -side top\n"
"\n"
"    # live widget updates on OSX in lieu of Apply button\n"
"    if {$::windowingsystem eq \"aqua\"} {\n"
"\n"
"        # call apply on Return in entry boxes that are in focus & rebind Return to ok button\n"
"        bind $mytoplevel.para.knobstyle.ticks.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.para.knobstyle.discrete.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.para.knobstyle.start.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.para.knobstyle.end.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"\n"
"        # unbind Return from ok button when an entry takes focus\n"
"        $mytoplevel.para.knobstyle.ticks.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.para.knobstyle.start.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.para.knobstyle.end.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"    }\n"
"    set applycmd \"\"\n"
"    if {$::windowingsystem eq \"aqua\"} {\n"
"        set applycmd \"::dialog_knob::apply $mytoplevel\"\n"
"    }\n"
"\n"
"    # messages\n"
"    labelframe $mytoplevel.s_r -borderwidth 1 -padx 5 -pady 5 -text [_ \"Send/Receive\"]\n"
"    pack $mytoplevel.s_r -side top -fill x\n"
"    frame $mytoplevel.s_r.send\n"
"    pack $mytoplevel.s_r.send -side top -anchor e -padx 5\n"
"    label $mytoplevel.s_r.send.lab -text [_ \"Send symbol:\"]\n"
"    entry $mytoplevel.s_r.send.ent -textvariable ::dialog_knob::var_snd($vid) -width 21\n"
"    if { $snd ne \"nosndno\" } {\n"
"        pack $mytoplevel.s_r.send.lab $mytoplevel.s_r.send.ent -side left \\\n"
"            -fill x -expand 1\n"
"    }\n"
"\n"
"    frame $mytoplevel.s_r.receive\n"
"    pack $mytoplevel.s_r.receive -side top -anchor e -padx 5\n"
"    label $mytoplevel.s_r.receive.lab -text [_ \"Receive symbol:\"]\n"
"    entry $mytoplevel.s_r.receive.ent -textvariable ::dialog_knob::var_rcv($vid) -width 21\n"
"    if { $rcv ne \"norcvno\" } {\n"
"        pack $mytoplevel.s_r.receive.lab $mytoplevel.s_r.receive.ent -side left \\\n"
"            -fill x -expand 1\n"
"    }\n"
"\n"
"    # colors\n"
"    labelframe $mytoplevel.colors -borderwidth 1 -text [_ \"Colors\"] -padx 5 -pady 5\n"
"    pack $mytoplevel.colors -fill x\n"
"\n"
"    frame $mytoplevel.colors.select\n"
"    pack $mytoplevel.colors.select -side top\n"
"    radiobutton $mytoplevel.colors.select.radio0 \\\n"
"        -value 0 -variable ::dialog_knob::var_colortype($vid) \\\n"
"        -text [_ \"Background\"] -justify left\n"
"    radiobutton $mytoplevel.colors.select.radio1 \\\n"
"        -value 1 -variable ::dialog_knob::var_colortype($vid) \\\n"
"        -text [_ \"Front\"] -justify left\n"
"    if { $::dialog_knob::var_color_foreground($vid) ne \"none\" } {\n"
"        pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio1 -side left \\\n"
"    } else {\n"
"        pack $mytoplevel.colors.select.radio0  pack $mytoplevel.colors.select.radio1 -side left\n"
"    }\n"
"\n"
"    frame $mytoplevel.colors.sections\n"
"    pack $mytoplevel.colors.sections -side top\n"
"    button $mytoplevel.colors.sections.but -text [_ \"Compose color\"] \\\n"
"        -command \"::dialog_knob::choose_col_bkfrlb $mytoplevel\"\n"
"    pack $mytoplevel.colors.sections.but -side left -anchor w -pady 5 \\\n"
"        -expand yes -fill x\n"
"    frame $mytoplevel.colors.sections.exp\n"
"    # color scheme by Mary Ann Benedetto http://piR2.org\n"
"    foreach r {r1 r2 r3} hexcols {\n"
"       { \"#FFFFFF\" \"#DFDFDF\" \"#BBBBBB\" \"#FFC7C6\" \"#FFE3C6\" \"#FEFFC6\" \"#C6FFC7\" \"#C6FEFF\" \"#C7C6FF\" \"#E3C6FF\" }\n"
"       { \"#9F9F9F\" \"#7C7C7C\" \"#606060\" \"#FF0400\" \"#FF8300\" \"#FAFF00\" \"#00FF04\" \"#00FAFF\" \"#0400FF\" \"#9C00FF\" }\n"
"       { \"#404040\" \"#202020\" \"#000000\" \"#551312\" \"#553512\" \"#535512\" \"#0F4710\" \"#0E4345\" \"#131255\" \"#2F004D\" } } \\\n"
"    {\n"
"       frame $mytoplevel.colors.$r\n"
"       pack $mytoplevel.colors.$r -side top\n"
"       foreach i { 0 1 2 3 4 5 6 7 8 9} hexcol $hexcols \\\n"
"           {\n"
"               label $mytoplevel.colors.$r.c$i -background $hexcol -activebackground $hexcol -relief ridge -padx 7 -pady 0 -width 1\n"
"               bind $mytoplevel.colors.$r.c$i <Button> \"::dialog_knob::preset_col $mytoplevel $hexcol\"\n"
"           }\n"
"       pack $mytoplevel.colors.$r.c0 $mytoplevel.colors.$r.c1 $mytoplevel.colors.$r.c2 $mytoplevel.colors.$r.c3 \\\n"
"           $mytoplevel.colors.$r.c4 $mytoplevel.colors.$r.c5 $mytoplevel.colors.$r.c6 $mytoplevel.colors.$r.c7 \\\n"
"           $mytoplevel.colors.$r.c8 $mytoplevel.colors.$r.c9 -side left\n"
"    }\n"
"\n"
"    # buttons\n"
"    frame $mytoplevel.cao -pady 10\n"
"    pack $mytoplevel.cao -side top\n"
"    button $mytoplevel.cao.cancel -text [_ \"Cancel\"] \\\n"
"        -command \"::dialog_knob::cancel $mytoplevel\"\n"
"    pack $mytoplevel.cao.cancel -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
"    if {$::windowingsystem ne \"aqua\"} {\n"
"        button $mytoplevel.cao.apply -text [_ \"Apply\"] \\\n"
"            -command \"::dialog_knob::apply $mytoplevel\"\n"
"        pack $mytoplevel.cao.apply -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
"    }\n"
"    button $mytoplevel.cao.ok -text [_ \"OK\"] \\\n"
"        -command \"::dialog_knob::ok $mytoplevel\" -default active\n"
"    pack $mytoplevel.cao.ok -side left -expand 1 -fill x -padx 15 -ipadx 10\n"
"\n"
"\n"
"    # live widget updates on OSX in lieu of Apply button\n"
"    if {$::windowingsystem eq \"aqua\"} {\n"
"\n"
"        # call apply on Return in entry boxes that are in focus & rebind Return to ok button\n"
"        bind $mytoplevel.para.knobstyle.dim.w_ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.rng.logmode.expmode_entry <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.rng.range.min.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.rng.range.max_ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.s_r.send.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"        bind $mytoplevel.s_r.receive.ent <KeyPress-Return> \"::dialog_knob::apply_and_rebind_return $mytoplevel\"\n"
"\n"
"        # unbind Return from ok button when an entry takes focus\n"
"        $mytoplevel.para.knobstyle.dim.w_ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.rng.logmode.expmode_entry config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.rng.range.min.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.rng.range.max_ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.s_r.send.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"        $mytoplevel.s_r.receive.ent config -validate focusin -vcmd \"::dialog_knob::unbind_return $mytoplevel\"\n"
"\n"
"        # remove cancel button from focus list since it's not activated on Return\n"
"        $mytoplevel.cao.cancel config -takefocus 0\n"
"\n"
"        # show active focus on the ok button as it *is* activated on Return\n"
"        $mytoplevel.cao.ok config -default normal\n"
"        bind $mytoplevel.cao.ok <FocusIn> \"$mytoplevel.cao.ok config -default active\"\n"
"        bind $mytoplevel.cao.ok <FocusOut> \"$mytoplevel.cao.ok config -default normal\"\n"
"\n"
"        # since we show the active focus, disable the highlight outline\n"
"        $mytoplevel.cao.ok config -highlightthickness 0\n"
"        $mytoplevel.cao.cancel config -highlightthickness 0\n"
"    }\n"
"\n"
"\n"
"    position_over_window $mytoplevel $::focused_window\n"
"}\n"
"\n"
"# for live widget updates on OSX\n"
"proc ::dialog_knob::apply_and_rebind_return {mytoplevel} {\n"
"    ::dialog_knob::apply $mytoplevel\n"
"    bind $mytoplevel <KeyPress-Return> \"::dialog_knob::ok $mytoplevel\"\n"
"    focus $mytoplevel.cao.ok\n"
"    return 0\n"
"}\n"
"\n"
"# for live widget updates on OSX\n"
"proc ::dialog_knob::unbind_return {mytoplevel} {\n"
"    bind $mytoplevel <KeyPress-Return> break\n"
"    return 1\n"
"}\n"

"\n"
"proc ::dialog_knob::popupmenu_strval {path varname values labels {command {}}} {\n"
"    upvar 1 $varname var\n"
"\n"
"    menubutton ${path} -menu ${path}.menu -indicatoron 1 -relief raised \\\n"
"        -text [lindex $labels [lsearch -exact $values $var]]\n"
"    menu ${path}.menu -tearoff 0\n"
"    set idx 0\n"
"    foreach l $labels {\n"
"        $path.menu add radiobutton -label \"$l\" -variable $varname -value [lindex $values $idx]\n"
"        $path.menu entryconfigure last -command \"\\{$path\\} configure -text \\{$l\\}; $command\"\n"
"        incr idx\n"
"    }\n"
"}\n"
"\n"
);
