# ParaView example — generate a plugin's property panel XML

Generates the ParaView **Server Manager XML** for a filter's property panel from an annotated C++ struct — no hand-written XML, so the GUI can't drift from the setters. The struct (`threshold.h`) is a rosetta *spec* for VTK's `vtkThreshold` filter and exercises **every** capability of the ParaView backend.

## Files

| File | Role |
|---|---|
| `threshold.h` | the annotated spec for `vtkThreshold` — the single source of truth |
| `main.cpp` | calls `rosetta::to_paraview_xml<Threshold>()`, writes XML to stdout |
| `CMakeLists.txt` | clang-p2996 wiring |

## What each annotation produces

| In `threshold.h` | In the generated XML |
|---|---|
| `paraview_proxy{"vtkThreshold","filters","…"}` (on the class) | `<SourceProxy class="vtkThreshold" …>` in `<ProxyGroup name="filters">` |
| `paraview_input{"Input","vtkDataSet"}` (on the class) | `<InputProperty>` with ProxyGroup / DataType / InputArray domains |
| `paraview_array{"Scalars","Input"}` on `scalars` | `<StringVectorProperty command="SetInputArrayToProcess">` + `<ArrayListDomain>` |
| `enum ThresholdFunction` field | `<IntVectorProperty>` + `<EnumerationDomain>` (`Between`/`Upper`/`Lower`) |
| `range{0.0,1.0}` on a `double` | `<DoubleRangeDomain min max>` |
| `range{0,8}` on an `int` | `<IntRangeDomain min max>` |
| `bool allScalars` | `<IntVectorProperty>` + `<BooleanDomain>` |
| `combobox{{"Selected","All","Any"}}` on a string | `<StringListDomain>` |
| `readonly` on `outputCellCount` | `information_only="1"` + `command="GetOutputCellCount"` |
| a member initializer (e.g. `= 1.0`) | `default_values="1"` |
| `doc{"…"}` | `<Documentation>` |

Field names become VTK commands by convention: `lowerThreshold` → `SetLowerThreshold`.

## Build & run

```bash
cmake -G Ninja -B build && cmake --build build

./build/paraview_xml                   # print the Server Manager XML to stdout
./build/paraview_xml > Threshold.xml   # capture it
```

## Sample output (excerpt)

```xml
<SourceProxy name="Threshold" class="vtkThreshold" label="Threshold (rosetta)">
  <InputProperty name="Input" command="SetInputConnection"> … </InputProperty>
  <StringVectorProperty name="scalars" command="SetInputArrayToProcess"
                        number_of_elements="5" element_types="0 0 0 0 2">
    <ArrayListDomain name="array_list" attribute_type="Scalars" input_domain_name="input_array">
      <RequiredProperties><Property name="Input" function="Input"/></RequiredProperties>
    </ArrayListDomain>
  </StringVectorProperty>
  <IntVectorProperty name="thresholdFunction" command="SetThresholdFunction"
                     number_of_elements="1" default_values="0">
    <EnumerationDomain name="enum">
      <Entry value="0" text="Between"/><Entry value="1" text="Upper"/><Entry value="2" text="Lower"/>
    </EnumerationDomain>
  </IntVectorProperty>
  <DoubleVectorProperty name="lowerThreshold" command="SetLowerThreshold"
                        number_of_elements="1" default_values="0">
    <DoubleRangeDomain name="range" min="0" max="1"/>
  </DoubleVectorProperty>
  …
</SourceProxy>
```

## Using it in ParaView

This is the Server Manager XML half of a plugin. Pair it with a `vtkThreshold` subclass exposing the matching `Set*`/`Get*` methods and reference the XML from `paraview_add_plugin(... SERVER_MANAGER_XML Threshold.xml)`; ParaView builds the property panel from it. Editing a setter in C++? Re-annotate and regenerate — the panel follows automatically.

## Notes / current scope

- The "property half": one input port and array-selection are supported; multiple input ports and multi-component (tuple) properties are not yet generated.
- `paraview_proxy` / `paraview_input` / `paraview_array` live in [`<rosetta/paraview.h>`](../../include/rosetta/paraview.h) — the generic core knows nothing about ParaView; the backend reads these via the type-erased annotation channel.
