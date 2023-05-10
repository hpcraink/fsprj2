import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions, defaultPanelOptions } from 'types';
import * as d3 from 'd3';
interface Props extends PanelProps<PanelOptions> {}

export const ForceFeedbackPanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    //sessionStorage.clear(); //Clear to erase beforehand data
    let addingNodes = false;
    let links: any = [];
    let ProcessIndex = 0;
    // We want old Nodes? Doesnt work w/o
    // if (JSON.parse(sessionStorage.getItem('nodes')!) !== null) {
    //   nodes = JSON.parse(sessionStorage.getItem('nodes')!);
    // }

    function addNodes(_callback: any) {
      let nodeNumber = nodes.length;
      //From Drilldownfilename or from Settings, also implement default values?
      let nodeProcess: any = defaultPanelOptions.ProcessID;
      //Find Index of Process
      let nodeThreads: any = [];
      for (let index = 0; index < data.series.length / 2; index++) {
        if (data.series[index].fields[1].labels?.processid === nodeProcess.toString()) {
          ProcessIndex = index;
          break;
        }
      }
      //find all related threads
      while (data.series[ProcessIndex].fields[1].labels?.processid === nodeProcess.toString()) {
        nodeThreads.push(data.series[ProcessIndex].fields[1].labels?.thread);
        ProcessIndex++;
      }

      //Node for Process
      //TODO Add Writtenbytes
      if (!nodes.map((a: any) => a.name).includes(nodeProcess)) {
        nodes.push({ index: nodeNumber, name: nodeProcess.toString(), r: 10, writtenBytes: 0 });
        nodeNumber += 1;
        addingNodes = true;
      }

      //Show all Threads of the Process
      //TODO add writtenbytes
      for (let i = 1; i < nodeThreads.length + 1; i++) {
        if (!nodes.map((a: any) => a.name).includes(nodeThreads[i - 1])) {
          nodes.push({ index: nodeNumber, name: nodeThreads[i - 1], r: 5, writtenBytes: 0 });
          nodeNumber += 1;
          addingNodes = true;
        }
      }
      //TODO Later Add Filename for each Thread

      _callback();
    }

    //TODO Add actual written Bytes
    function changeNodeSize(node: any, value: any) {
      let maxNodeSize = 30;
      let minNodeSize = 10;
      let maxWrittenBytes = 10000; //TODO rel Wert?
      node.writtenBytes += value; //TODO wieso aufsummieren?
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
      for (let i = 0; i < nodes.length; i++) {
        console.log(i, 'Links:', links);
        let source = nodes[0];
        let target = nodes[i];
        let writtenBytes = data.series[nodes.length - ProcessIndex + i].fields[1].values.get(0);
        //TODO Add written Bytes for Process aswell
        changeNodeSize(nodes[target.index], writtenBytes);
        //TODO Change/delete Link Colour | not needed anymore? Colour by Filesystem? => Add legend?
        let link_color = '#003f5c';
        if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
          link_color = '#ef5675';
        } else if (data.series[i].fields[1].labels?.functionname === 'write') {
          link_color = '#ffa600';
        } else if (data.series[i].fields[1].labels?.functionname === 'writev') {
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
