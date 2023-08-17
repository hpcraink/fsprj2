import { PanelPlugin } from '@grafana/data';
import { PanelOptions, defaultPanelOptions } from './types';
import { ForceFeedbackPanel } from './ForcePanel';

export const plugin = new PanelPlugin<PanelOptions>(ForceFeedbackPanel).setPanelOptions((builder) => {
  return builder
    .addBooleanSwitch({
      path: 'UseFilterFilename',
      name: 'Warning:',
      description: 'Not using the filter a lead to a messy/unreadable Forcegraph due to too many Datapoints.',
      defaultValue: defaultPanelOptions.UseFilterFilename,
    })
    .addTextInput({
      path: 'FilterFilename',
      name: 'Only show Filenames containing:',
      description: "Plugin will show Process with related Threads and Filenames. Split Inputs with ','.",
      defaultValue: defaultPanelOptions.FilterFilename,
    })
    .addBooleanSwitch({
      path: 'UseNegativeFilterFilename',
      name: '',
      defaultValue: defaultPanelOptions.UseNegativeFilterFilename,
    })
    .addTextInput({
      path: 'NegFilterFilename',
      name: 'Filter Filenames containing:',
      description: "Plugin will show Process with related Threads and Filenames. Split Inputs with ','.",
      defaultValue: defaultPanelOptions.NegFilterFilename,
    })
    .addBooleanSwitch({
      path: 'UseWrite',
      name: 'Show written Data.',
      defaultValue: defaultPanelOptions.UseWrite,
    })
    .addBooleanSwitch({
      path: 'UseRead',
      name: 'Show read Data.',
      defaultValue: defaultPanelOptions.UseRead,
    });
});
