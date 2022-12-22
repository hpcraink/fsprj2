import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';
import * as d3 from 'd3';
interface Props extends PanelProps<SimpleOptions> {}
export const ForceFeedbackPanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    let links: any = [];

    const values = data.series
      .map((series) => series.fields.find((field) => field.type === 'number'))
      .map((field) => field?.values.get(0));

    function addNodes(_callback: any) {
      let nodeNumber = 0;
      for (let i = 0; i < data.series.length; i++) {
        if (!nodes.map((a: any) => a.name).includes(data.series[i].fields[1].labels?.hostname)) {
          nodes.push({ index: nodeNumber, name: data.series[i].fields[1].labels?.hostname, r: 10, writtenBytes: 0 });
          nodeNumber += 1;
        }
        if (!nodes.map((a: any) => a.name).includes(data.series[i].fields[1].labels?.thread)) {
          nodes.push({ index: nodeNumber, name: data.series[i].fields[1].labels?.thread, r: 5, writtenBytes: 0 });
          nodeNumber += 1;
        }
      }
      _callback();
    }

    function addLinks() {
      console.log(nodes);
      for (let i = 0; i < data.series.length; i++) {
        let source = nodes.find((a: { name: any }) => a.name === data.series[i].fields[1].labels?.thread);
        let target = nodes.find((a: { name: any }) => a.name === data.series[i].fields[1].labels?.hostname);

        let link_color = '#003f5c';
        if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
          link_color = '#ef5675';
        } else if (data.series[i].fields[1].labels?.functionname === 'write') {
          link_color = '#ffa600';
        }

        if (links.length === 0) {
          links.push({
            source: source.index,
            target: target.index,
            color: link_color,
          });
        } else {
          if (
            links.map((a: any) => a.source).includes(source.index) &&
            links.map((a: any) => a.target).includes(target.index)
          ) {
            //pass
          } else {
            links.push({
              source: source.index,
              target: target.index,
              color: link_color,
            });
          }
        }
      }
    }

    function forceSimulation(_callback: any) {
      let simulation = d3
        .forceSimulation(nodes)
        .force('link', d3.forceLink().links(links).distance(75).strength(4))
        .force('charge', d3.forceManyBody().strength(-8))
        .stop();

      simulation.tick(500);
      _callback();
    }

    function drawForceGraph() {
      const margin = { left: 30, top: 30, right: 30, bottom: 30 };
      const chartWidth = width - (margin.left + margin.right);
      const chartHeight = height - (margin.top + margin.bottom);
      console.log(links);

      let xBorder: [any, any] = d3.extent(nodes, function (d: any) {
        return +d.x;
      });
      let yBorder: [any, any] = d3.extent(nodes, function (d: any) {
        return +d.y;
      });

      const xScale = d3
        .scaleLinear()
        .domain([xBorder[0] - 50, xBorder[1] + 50])
        .range([0, chartWidth]);
      const yScale = d3
        .scaleLinear()
        .domain([yBorder[0] - 50, yBorder[1] + 50])
        .range([chartHeight, 0]);

      const svg = d3.select('#area');
      svg.selectAll('*').remove();
      svg
        .selectAll('line')
        .data(links)
        .enter()
        .append('line')
        .attr('x1', function (d: any) {
          return xScale(d.source.x);
        })
        .attr('y1', function (d: any) {
          return yScale(d.source.y);
        })
        .attr('x2', function (d: any) {
          return xScale(d.target.x);
        })
        .attr('y2', function (d: any) {
          return yScale(d.target.y);
        })
        .attr('stroke', function (d: any) {
          return d.color;
        })
        .attr('stroke-width', 1.5);
      svg
        .selectAll('circle')
        .data(nodes)
        .enter()
        .append('circle')
        .attr('cx', function (d: any) {
          return xScale(d.x);
        })
        .attr('cy', function (d: any) {
          return yScale(d.y);
        })
        .attr('r', function (d: any) {
          return d.r;
        })
        .style('fill', '#888888');
    }

    function runSimulation() {
      addNodes(() => addLinks());
      forceSimulation(() => drawForceGraph());
    }

    if (values[0] === undefined) {
      d3.select('p').text('No data');
    } else {
      runSimulation();
    }
  }, [data, height, width]);
  return (
    <div className="App">
      <p></p>
      <svg id="area" height={height} width={width}></svg>
    </div>
  );
};
