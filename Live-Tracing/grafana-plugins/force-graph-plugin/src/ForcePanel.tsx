import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';
import * as d3 from 'd3';
interface Props extends PanelProps<SimpleOptions> {}
export const ForceFeedbackPanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    let links: any = [];

    const threads = data.series
      .map((series) => series.fields.find((field) => field.type === 'number'))
      .map((field) => field?.values.get(0));

    /*
    function getRandomArbitrary(min: number, max: number) {
      return Math.random() * (max - min) + min;
    }

    let threads = [
      getRandomArbitrary(20, 40),
      getRandomArbitrary(20, 60),
      getRandomArbitrary(30, 80),
      getRandomArbitrary(20, 80),
    ];
*/
    let threadPos = [
      { x: -100, y: -100 },
      { x: -100, y: 100 },
      { x: 100, y: -100 },
      { x: 100, y: 100 },
    ];
    console.log(threads);
    let nodeNumber = 0;
    let saveNode = 0;
    //for (let i = 0; i < threads.length; i++) {
    for (let i = 0; i < 4; i++) {
      nodes.push({ index: nodeNumber, r: 10, fx: threadPos[i].x, fy: threadPos[i].y });
      saveNode = nodeNumber;
      nodeNumber += 1;
      for (let j = 0; j < threads[i]; j++) {
        nodes.push({ index: nodeNumber, r: 4 });
        links.push({ source: nodeNumber, target: saveNode });
        nodeNumber += 1;
      }
    }

    function forceSimulation(_callback: any) {
      let simulation = d3
        .forceSimulation(nodes)
        .force('link', d3.forceLink().links(links).distance(50))
        .force('charge', d3.forceManyBody().strength(-2))
        //        .force('collide', d3.forceCollide().radius(10))
        .stop();

      simulation.tick(1000);
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
        .attr('stroke', 'blue')
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
        .style('fill', 'green');
    }

    function runSimulation() {
      forceSimulation(() => drawForceGraph());
    }

    if (threads[0] === undefined) {
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
