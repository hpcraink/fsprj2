import { PanelPlugin } from '@grafana/data';
import { PanelOptions, defaultPanelOptions } from './types';
import { ForceFeedbackPanel } from './ForcePanel';

export const plugin = new PanelPlugin<PanelOptions>(ForceFeedbackPanel).setPanelOptions((builder) => {
  return builder.addNumberInput({
    path: 'ProcessID',
    name: 'Add the ProcessID for Drilldown below:',
    description: 'Plugin will show Process with related Threads and the Filenames in which the threads write',
    defaultValue: defaultPanelOptions.ProcessID,
  });
});
