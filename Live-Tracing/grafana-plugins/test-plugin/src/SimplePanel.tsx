import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';
import * as d3 from 'd3';
import { openDB, DBSchema } from 'idb';

interface Props extends PanelProps<SimpleOptions> {}

interface MyDB extends DBSchema {
  nodeNr: {
    key: string;
    value: number;
  };
  nodes: {
    key: string;
    value: {
      index: number;
      r: number;
      x: number;
      y: number;
      vx: number;
      vy: number;
    };
  };
  links: {
    key: string;
    value: {
      source: number;
      target: number;
    };
  };
}

export function initDB() {
  openDB<MyDB>('db', 1, {
    upgrade(db) {
      db.createObjectStore('nodeNr');
      db.createObjectStore('nodes', {
        keyPath: 'index',
      });
      db.createObjectStore('links', { autoIncrement: true });
    },
  });
}

export async function storeNodeNr(nodeNumber: number) {
  const db = await openDB('db', 1);
  db.put('nodeNr', nodeNumber, 'nodeNr');
  db.close();
}
export async function storeNodes(nodes: any) {
  const db = await openDB('db', 1);
  for (const node of nodes) {
    db.put('nodes', { index: node.index, r: node.r, x: node.x, y: node.y, vx: node.vx, vy: node.vy });
  }
  db.close();
}
export async function storeLinks(links: any) {
  const db = await openDB('db', 1);
  for (const link of links) {
    db.put('links', { source: link.source, target: link.target });
  }
  db.close();
}

export async function getNodeNr() {
  const db = await openDB('db', 1);
  let result = await db.get('nodeNr', 'nodeNr');
  return Number(result);
}

export const SimplePanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    let links: any = [];
    let initLinks: any = [];
    let nodeNumber = Number(sessionStorage.getItem('nodeNr'));
    let savedLinks = JSON.parse(sessionStorage.getItem('saveLinks')!);
    let graphTest = sessionStorage.getItem('graph');
    let graphTmp = JSON.parse(graphTest!);

    function initGraph() {
      initDB();
      nodes.push({ index: 0, r: 10, fx: 0, fy: 0 });
      for (let i = 0; i < 4; i++) {
        nodes.push({ index: i + 1, r: 5 });
        links.push({ source: 0, target: i + 1 });
        initLinks.push({ source: 0, target: i + 1 });
        nodeNumber = i + 1;
      }
    }
    function updateGraph() {
      initLinks = savedLinks;
      nodes = graphTmp.graphNodes;
      links = graphTmp.graphLinks;
      nodeNumber = nodeNumber!;

      for (let i = 0; i < 2; i++) {
        nodes.push({ index: nodeNumber, r: 5 });
        links.push({ source: 0, target: nodeNumber + 1 });
        initLinks.push({ source: 0, target: nodeNumber + 1 });
        nodeNumber += 1;
      }
    }
    function forceSimulation(_callback: any) {
      let simulation = d3
        .forceSimulation(nodes)
        .force('link', d3.forceLink().links(links).distance(50))
        .force('charge', d3.forceManyBody().strength(-100))
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
    function storeGraph() {
      let graphTmp = {
        graphNodes: nodes,
        graphLinks: initLinks,
      };
      sessionStorage.setItem('nodeNr', JSON.stringify(nodeNumber));
      sessionStorage.setItem('saveLinks', JSON.stringify(initLinks));
      sessionStorage.setItem('graph', JSON.stringify(graphTmp));
      storeNodeNr(nodeNumber);
      storeLinks(initLinks);
      storeNodes(nodes);
    }
    function runSimulation() {
      forceSimulation(() => drawForceGraph());
      storeGraph();
    }
    if (graphTmp == null) {
      d3.select('p').text('No data');
      initGraph();
      runSimulation();
    } else {
      updateGraph();
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
