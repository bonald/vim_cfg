#!/bin/bash
SDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


cp standard/ietf/RFC/ietf-yang-types.yang  $SDIR/yang/.
cp standard/ietf/RFC/ietf-inet-types.yang  $SDIR/yang/.
cp standard/ietf/RFC/ietf-interfaces.yang  $SDIR/yang/.

cp types/openconfig-inet-types.yang $SDIR/yang/.
cp types/openconfig-types.yang $SDIR/yang/.
cp types/openconfig-yang-types.yang $SDIR/yang/.
cp interfaces/openconfig-interfaces.yang  $SDIR/yang/.
cp interfaces/openconfig-if-ethernet.yang  $SDIR/yang/.
cp interfaces/openconfig-if-aggregate.yang  $SDIR/yang/.
cp openconfig-extensions.yang  $SDIR/yang/.
cp policy/openconfig-policy-types.yang  $SDIR/yang/.
cp vlan/openconfig-vlan-types.yang $SDIR/yang/.

cp vlan/openconfig-vlan.yang $SDIR/yang/.
cp local-routing/openconfig-local-routing.yang  $SDIR/yang/.



PYBINDPLUGIN=`/usr/bin/env python -c 'import pyangbind; import os; print "%s/plugin" % os.path.dirname(pyangbind.__file__)'`
pyang --plugindir $PYBINDPLUGIN -f pybind  -p yang --build-rpcs --split-class-dir $SDIR/../python_module/oc_lr  $SDIR/yang/openconfig-local-routing.yang
#pyang --plugindir $PYBINDPLUGIN -f pybind  -p yang --build-rpcs --split-class-dir $SDIR/../python_module/oc_vlan $SDIR/yang/openconfig-vlan.yang
pyang --plugindir $PYBINDPLUGIN -f pybind  -p yang --build-rpcs --split-class-dir $SDIR/../python_module/oc_if $SDIR/yang/openconfig-interfaces.yang

echo "Bindings successfully generated!"
