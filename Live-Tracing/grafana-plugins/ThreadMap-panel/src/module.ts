import { PanelPlugin } from '@grafana/data';
import { defaultPanelOptions, PanelOptions } from 'options';
import { ThreadMap } from './ThreadMap';

export const plugin = new PanelPlugin<PanelOptions>(ThreadMap).setPanelOptions(builder => {
  let category = ['ThreadMap']
  builder
  .addBooleanSwitch({
    path: 'UseMinMaxBoolean',
    name: 'Use specified min/max Values',
    defaultValue: false,
    category,
  })
    .addNumberInput({
      path: 'ThreadMapColor.min',
      name: 'Min.value',
      defaultValue: defaultPanelOptions.minmax.min,
      settings: {
        placeholder: 'Auto',
      },
      category,
    })
    .addNumberInput({
      path: 'ThreadMapColor.max',
      name: 'Max.value',
      defaultValue: defaultPanelOptions.minmax.max,
      settings: {
        placeholder: 'Auto',
      },
      category,
    })
  
  category = ['ONLY FOR TETSTING! Y Axis']

    builder
      .addUnitPicker({
        category,
        path: 'yAxis.unit',
        name: 'Unit',
        defaultValue: undefined,
        settings: {
          isClearable: true,
        },
      })
      .addNumberInput({
        category,
        path: 'yAxis.decimals',
        name: 'Decimals',
        settings: {
          placeholder: 'Auto',
        },
      });

      if (!false) {
        // if undefined, then show the min+max
        builder
          .addNumberInput({
            path: 'yAxis.min',
            name: 'Min value',
            settings: {
              placeholder: 'Auto',
            },
            category,
          })
          .addTextInput({
            path: 'yAxis.max',
            name: 'Max value',
            settings: {
              placeholder: 'Auto',
            },
            category,
          });
      }
})
