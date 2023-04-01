//options are used for the manual configuartion of the ThreadMap, also in module

//Options to Panel
export interface PanelOptions {
    UseMinMaxBoolean: any;
    ThreadMapColor: { min: any; max: any; }
}
 
export const defaultPanelOptions: PanelOptions = {
    ThreadMapColor: {
        min: "no data",
        max: "no data", //series max value
    },
    UseMinMaxBoolean: false
};
