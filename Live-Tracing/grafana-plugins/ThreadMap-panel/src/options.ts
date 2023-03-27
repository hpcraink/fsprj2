//options are used for the manual configuartion of the ThreadMap, also in module
//import { ColAssig } from './ThreadMap';
//import { minValue } from "ThreadMap";

export interface ThreadMapColor {
    min: number
    max: number
}

//Following is for tests

//delete until here

export interface PanelOptions {
    minmax: ThreadMapColor
}
 
export const defaultPanelOptions: PanelOptions = {
    minmax: {
        min: 20,     //series min value
        max: 1000   //series max value
    }
};

