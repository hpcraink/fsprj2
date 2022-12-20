 import { AxisConfig, AxisPlacement, HideableFieldConfig, ScaleDistributionConfig, VisibilityMode } from '@grafana/schema';


export const modelVersion = Object.freeze([1, 0]);

export enum ThreadMapColorMode {
  Opacity = 'opacity',
  Scheme = 'scheme',
}

export enum ThreadMapColorScale {
  Linear = 'linear',
  Exponential = 'exponential',
}

export interface ThreadMapColorOptions {
  mode: ThreadMapColorMode;
  scheme: string; // when in scheme mode -- the d3 scheme name
  fill: string; // when opacity mode, the target color
  scale: ThreadMapColorScale; // for opacity mode
  exponent: number; // when scale== sqrt
  steps: number; // 2-128

  reverse: boolean;

  // Clamp the colors to the value range
  min?: number;
  max?: number;
}
export interface YAxisConfig extends AxisConfig {
  unit?: string;
  reverse?: boolean;
  decimals?: number;
  // Only used when the axis is not ordinal
  min?: number;
  max?: number;
}

export interface FilterValueRange {
  le?: number;
  ge?: number;
}


export interface PanelOptions {
  showSeriesCount: boolean;
  calculate?: boolean;

  color: ThreadMapColorOptions;
  filterValues?: FilterValueRange; // was hideZeroBuckets
  showValue: VisibilityMode;
  yAxis: YAxisConfig;
}

export const defaultPanelOptions: PanelOptions = {
    showSeriesCount: true,
    calculate: false,
    color: {
        mode: ThreadMapColorMode.Scheme,
        scheme: 'Oranges',
        fill: 'dark-orange',
        scale: ThreadMapColorScale.Exponential,
        reverse: false,
        exponent: 0.5,
        steps: 64,
    },
    yAxis: {
        axisPlacement: AxisPlacement.Left,
    },
    showValue: VisibilityMode.Auto,
    filterValues: {
        le: 1e-9,
    },
};

export interface PanelFieldConfig extends HideableFieldConfig {
  scaleDistribution?: ScaleDistributionConfig;
}

export const defaultPanelFieldConfig: PanelFieldConfig = {};