<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <!-- interface-requires gtk+ 2.12 -->
  <!-- interface-naming-policy project-wide -->
  <object class="GtkWindow" id="window">
    <property name="can_focus">False</property>
    <property name="title" translatable="yes">Nastavení Fingerpaint</property>
    <child>
      <object class="GtkFixed" id="fixed1">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <child>
          <object class="GtkHScale" id="hscale1">
            <property name="width_request">202</property>
            <property name="height_request">80</property>
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="show_fill_level">True</property>
            <property name="round_digits">1</property>
            <property name="digits">0</property>
          </object>
          <packing>
            <property name="x">12</property>
            <property name="y">9</property>
          </packing>
        </child>
        <child>
          <object class="GtkDrawingArea" id="canvas">
            <property name="width_request">640</property>
            <property name="height_request">480</property>
            <property name="visible">True</property>
            <property name="can_focus">False</property>
          </object>
          <packing>
            <property name="x">15</property>
            <property name="y">87</property>
          </packing>
        </child>
        <child>
          <object class="GtkLabel" id="label1">
            <property name="width_request">100</property>
            <property name="height_request">40</property>
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="yalign">0.43000000715255737</property>
            <property name="ypad">10</property>
            <property name="label" translatable="yes">výběr barvy</property>
          </object>
          <packing>
            <property name="x">437</property>
            <property name="y">26</property>
          </packing>
        </child>
        <child>
          <object class="GtkDrawingArea" id="drawingarea2">
            <property name="width_request">40</property>
            <property name="height_request">40</property>
            <property name="visible">True</property>
            <property name="can_focus">False</property>
          </object>
          <packing>
            <property name="x">553</property>
            <property name="y">29</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
</interface>
