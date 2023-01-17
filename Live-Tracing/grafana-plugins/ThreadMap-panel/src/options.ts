//options are used for the manual configuartion of the ThreadMap, also in module


export interface ThreadMapColor {
    min?: number
    max?: number
}

//Following is for tests


//delete until here

export interface PanelOptions {
  minmax: ThreadMapColor
}

export const defaultPanelOptions: PanelOptions = {
    minmax: {
        min: 0,
        max: 100
    }
};