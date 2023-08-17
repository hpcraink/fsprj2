import { PanelPlugin } from '@grafana/data';
import {PanelOptions, defaultPanelOptions } from 'options';
import { ThreadMap } from './ThreadMap';

export const plugin = new PanelPlugin<PanelOptions>(ThreadMap).setPanelOptions(builder => {
  let category = ['ThreadMap']
  builder
  .addSelect({
    path: 'SortThreadMapBy',
    name: 'Sort By:',
    defaultValue: defaultPanelOptions.SortThreadMapBy[0],
    category,
    settings: {
      options: defaultPanelOptions.SortThreadMapBy.map((SortBy: string) =>
      ({
        value: SortBy,
        label: SortBy
      }))
    },
  })
  .addBooleanSwitch({
    path: 'UseMinMaxBoolean',
    name: 'Use specified min/max Values',
    defaultValue: defaultPanelOptions.UseMinMaxBoolean,
    category,
  })
  .addNumberInput({
    path: 'ThreadMapColor.min',
    name: 'MinValue',
    defaultValue: defaultPanelOptions.ThreadMapColor.min,
    settings: {
      placeholder: 'Auto',
    },
    category,
    showIf: (config) => config.UseMinMaxBoolean
  })
  .addNumberInput({
    path: 'ThreadMapColor.max',
    name: 'MaxValue',
    defaultValue: defaultPanelOptions.ThreadMapColor.max,
    settings: {
      placeholder: 'Auto',
    },
    category,
    showIf: (config) => config.UseMinMaxBoolean
    })
    .addBooleanSwitch({
      path: 'UseFileSystemFinder',
      name: 'Activate FileSystemFinder',
      defaultValue: defaultPanelOptions.UseFileSystemFinder,
      category,
    })
    .addTextInput({
      path: 'FileSystemFinder',
      name: 'FileSystemFinder:',
      description: '',
      defaultValue: defaultPanelOptions.FileSystemFinder,
      category,
    })

})
