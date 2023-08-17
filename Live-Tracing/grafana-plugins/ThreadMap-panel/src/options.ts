//options are used for the manual configuartion of the ThreadMap, also in module

//Options to Panel
export interface PanelOptions {
    UseMinMaxBoolean: any;
    ThreadMapColor: { min: any; max: any; };
    SortThreadMapBy: [string, string, string];
    UseFileSystemFinder: any;
    FileSystemFinder: string;
}
 
export const defaultPanelOptions: PanelOptions = {
    ThreadMapColor: {
        min: 'no data',
        max: 'no data', //series max value
    },
    UseMinMaxBoolean: false,
    SortThreadMapBy: [
        'default',
        'MaxBytes',
        'Filesystem'
    ],
    UseFileSystemFinder: false,
    FileSystemFinder: 'pfs',
};
