//options are used for the manual configuartion of the ThreadMap, also in module

//Options to Panel
export interface PanelOptions {
  UseFilterFilename: any;
  FilterFilename: string;
  UseNegativeFilterFilename: any;
  NegFilterFilename: string;
  UseWrite: any;
  UseRead: any;
}

export const defaultPanelOptions: PanelOptions = {
  UseFilterFilename: true,
  FilterFilename: 'pfs',
  UseNegativeFilterFilename: false,
  NegFilterFilename: '',
  UseWrite: true,
  UseRead: true,
};
