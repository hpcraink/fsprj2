import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';
import * as d3 from 'd3';
interface Props extends PanelProps<SimpleOptions> {}

export const ForceFeedbackPanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    //sessionStorage.clear(); //Clear to erase beforehand data | Took wrong data from false test before!
    let addingNodes = false;
    let links: any = [];
    if (JSON.parse(sessionStorage.getItem('nodes')!) !== null) {
      nodes = JSON.parse(sessionStorage.getItem('nodes')!);
    }

    function addNodes(_callback: any) {
      let nodeNumber = nodes.length;
      let nodeHosts: any = data.series[0].fields[1].values;
      let nodeThreads: any = data.series[1].fields[1].values;

      for (let i = 0; i < nodeHosts.length; i++) {
        if (!nodes.map((a: any) => a.name).includes(nodeHosts.get(i))) {
          nodes.push({ index: nodeNumber, name: nodeHosts.get(i), r: 10, writtenBytes: 0 });
          nodeNumber += 1;
          addingNodes = true;
        }
      }
      for (let i = 0; i < nodeThreads.length; i++) {
        if (!nodes.map((a: any) => a.name).includes(nodeThreads.get(i))) {
          nodes.push({ index: nodeNumber, name: nodeThreads.get(i).toString(), r: 5, writtenBytes: 0 });
          //nodes.push({ index: nodeNumber, name: nodeThreads.get(i), r: 5, writtenBytes: 0 });
          nodeNumber += 1;
          addingNodes = true;
        }
      }
      _callback();
    }

    function changeNodeSize(node: any, value: any) {
      let maxNodeSize = 30;
      let minNodeSize = 10;
      let maxWrittenBytes = 10000; //ToDo rel Wert?
      node.writtenBytes += value;
      let nodeSize = node.writtenBytes / maxWrittenBytes;
      if (nodeSize >= 1) {
        node.r = maxNodeSize;
      } else if (nodeSize > 0.33) {
        node.r = nodeSize * maxNodeSize;
      } else {
        node.r = minNodeSize;
      }
    }

    function addLinks() {
      links.length = 0;
      for (let i = 2; i < data.series.length; i++) {
        let source = nodes.find((a: { name: any }) => a.name === data.series[i].fields[1].labels?.thread);
        let target = nodes.find((a: { name: any }) => a.name === data.series[i].fields[1].labels?.hostname);
        let writtenBytes = data.series[i].fields[1].values.get(0); //Falsch, bekommt nicht alle written bytes des Threads nur buffer[0]
        changeNodeSize(nodes[target.index], writtenBytes); //didnt changed target.index to source.index
        let link_color = '#003f5c';
        if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
          link_color = '#ef5675';
        } else if (data.series[i].fields[1].labels?.functionname === 'write') {
          link_color = '#ffa600';
        } else if (data.series[i].fields[1].labels?.functionname === 'writev') {
          //writev not implemented before
          link_color = '#ff0000';
        }
        links.push({
          source: source.index,
          target: target.index,
          color: link_color,
        });
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

    function fixateNodes() {
      if (addingNodes === true) {
        for (let i = 0; i < nodes.length; i++) {
          if (nodes[i].x !== undefined && nodes[i].y !== undefined) {
            nodes[i].fx = nodes[i].x;
            nodes[i].fy = nodes[i].y;
          }
        }
      }
    }

    function runSimulation() {
      addNodes(() => addLinks());
      forceSimulation(() => drawForceGraph());
      fixateNodes();
      sessionStorage.setItem('nodes', JSON.stringify(nodes));
    }
    if (data.series[2].length === 0) {
      d3.select('#area').selectAll('*').remove();
      d3.select('p').text('No data');
      sessionStorage.clear();
    } else {
      d3.select('p').text('');
      d3.select('#area').selectAll('*').remove();
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
