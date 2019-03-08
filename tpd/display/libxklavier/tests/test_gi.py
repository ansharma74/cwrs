#!/usr/bin/python
#
# Test GI binding of libxklavier
# Copyright (C) 2011 Martin Pitt <martin.pitt@ubuntu.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

import sys
import os

# use source tree typelib
os.environ['GI_TYPELIB_PATH'] = 'libxklavier:' + os.environ.get('GI_TYPELIB_PATH', '')

from gi.repository import Xkl, Gdk, GdkX11

def item_str(s):
    '''Convert a zero-terminated byte array to a proper str'''

    i = s.find(b'\x00')
    return s[:i].decode()

display = GdkX11.x11_get_default_xdisplay()

print('== Engine ==')
engine = Xkl.Engine.get_instance(display)

print('indicator names:', engine.get_indicators_names())
print('group names:', engine.get_groups_names())
print('default layout:', engine.get_groups_names()[engine.get_default_group()])
print('features: %X' % engine.get_features())
st = engine.get_current_state()
print('current state: group: %i, indicators: %u' % (st.group, st.indicators))

# check ConfigItem ctor with data
i = Xkl.ConfigItem()
assert item_str(i.name) == ''
i.set_name('fr')
assert item_str(i.name) == 'fr'

# load registry
registry = Xkl.ConfigRegistry.get_instance(engine)

if not registry.load(False):
    print('Failed to load registry')
    sys.exit(1)

print('\n== Available Layouts ==')
def layout_iter(registry, item, data):
    print('[%s] %s, ' % (item_str(item.name), item_str(item.description)))

registry.foreach_layout(layout_iter, None)
print()

print('\n== ConfigRec ==')
rec = Xkl.ConfigRec()
if not rec.get_from_server(engine):
    print('Failed to get configuration from server')
    sys.exit(1)

print('Curent configuration:')
print('  Model:', rec.model)
print('  Layouts:', rec.layouts)
print('  Variants:', rec.variants)
print('  Options:', rec.options)

print('Adding Danish layout...')
rec.set_layouts(rec.layouts + ['dk'])
rec.set_variants(rec.variants + [''])
if not rec.activate(engine):
    print('Failed to activate new configuration')

print('Curent configuration:')
rec = Xkl.ConfigRec()
if not rec.get_from_server(engine):
    print('Failed to get configuration from server')
    sys.exit(1)
print('  Model:', rec.model)
print('  Layouts:', rec.layouts)
print('  Variants:', rec.variants)
print('  Options:', rec.options)

print('Removing Danish layout...')
rec.set_layouts(rec.layouts[:-1])
rec.set_variants(rec.variants[:-1])
if not rec.activate(engine):
    print('Failed to activate new configuration')

print('Curent configuration:')
rec = Xkl.ConfigRec()
if not rec.get_from_server(engine):
    print('Failed to get configuration from server')
    sys.exit(1)
print('  Model:', rec.model)
print('  Layouts:', rec.layouts)
print('  Variants:', rec.variants)
print('  Options:', rec.options)

print('Changing model to "pc105"...')
original_model = rec.model
rec.set_model("pc105")

print('Curent configuration:')
print('  Model:', rec.model)
print('  Layouts:', rec.layouts)
print('  Variants:', rec.variants)
print('  Options:', rec.options)

print('Changing back to original model...')
rec.set_model(original_model)

print('Curent configuration:')
print('  Model:', rec.model)
print('  Layouts:', rec.layouts)
print('  Variants:', rec.variants)
print('  Options:', rec.options)

