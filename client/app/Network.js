import inPolygon from 'point-in-polygon';
import Mousetrap from 'mousetrap';

const originPosition = ({x, y, svg}) => {
  let pt = svg.createSVGPoint();
  pt.x = x;
  pt.y = y;
  let svgP = pt.matrixTransform(svg.getScreenCTM().inverse());
  return {
    x: svgP.x, y: svgP.y
  }
};
const distance = (p1, p2) => {
  return Math.sqrt((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2);
};
const ccwAngle = (a, b) => {
  // from a to b
  let r = b - a;
  if (r < 0) return r + 360;
  else return r;
};
const findCenters = ({x1, y1, x2, y2, radius: r}) => {
  let q = Math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2);
  // midpoint
  let y3 = (y1 + y2) / 2;
  let x3 = (x1 + x2) / 2;

  return [
    {
      x: x3 + Math.sqrt(r ** 2 - (q / 2) ** 2) * (y1 - y2) / q,
      y: y3 + Math.sqrt(r ** 2 - (q / 2) ** 2) * (x2 - x1) / q
    },
    {
      x: x3 - Math.sqrt(r ** 2 - (q / 2) ** 2) * (y1 - y2) / q,
      y: y3 - Math.sqrt(r ** 2 - (q / 2) ** 2) * (x2 - x1) / q
    }
  ]
};
const processNeighbors = ({nodes, range}) => {
  let V = nodes.length;
  for (let node of nodes) {
    node.neighbors = [];
    node.neighborIds = [];
  }

  for (let i = 0; i < V; i++) {
    for (let j = i + 1; j < V; j++) {
      if ((nodes[i].x - nodes[j].x) ** 2 + (nodes[i].y - nodes[j].y) ** 2 <= range ** 2) {
        nodes[i].neighbors.push(nodes[j]);
        nodes[i].neighborIds.push(j);
        nodes[j].neighbors.push(nodes[i]);
        nodes[j].neighborIds.push(i);
      }
    }
  }

  return nodes;
};
const angle = ({x: x1, y: y1}, {x: x2, y: y2}) => {
  let ag = Math.atan2(y2 - y1, x2 - x1) * 180 / Math.PI;
  if (ag < 0) {
    ag += 360;
  }

  return ag;
};
const findBallCenter = ({node, ballDiameter}) => {
  let candidates = [];
  let radius = ballDiameter / 2;
  for (let neighbor of node.neighbors) {
    let {x: x1, y: y1} = node;
    let {x: x2, y: y2} = neighbor;

    for (let center of findCenters({x1, y1, x2, y2, radius})) {
      let ok = true;
      for (let otherNeighbor of node.neighbors) {
        if (otherNeighbor === neighbor) continue;
        if (distance(center, otherNeighbor) < radius) {
          ok = false;
        }
      }

      if (ok) {
        candidates.push(center);
      }
    }
  }

  candidates.sort((center1, center2) => angle(node, center1) - angle(node, center2));
  if (candidates.length) {
    return candidates[0];
  } else {
    return null;
  }
};

// function findCenterOnClick({nodes, ballDiameter, onFindCenter}) {
//   nodes.forEach(node => {
//     node.circle.off('click');
//     node.circle.click((e) => {
//       let center = findBallCenter({node, ballDiameter});
//       if (center) {
//         onFindCenter({center, node});
//       }
//     })
//   });
//
//   return nodes;
// }

function rotateVectorCCW({x, y, angle}) {
  return {
    x: x * Math.cos(angle) - y * Math.sin(angle),
    y: x * Math.sin(angle) + y * Math.cos(angle)
  }
}

function length({x, y}) {
  return Math.sqrt(x * x + y * y);
}


const NODE_CIRCLE_RADIUS = 3;

class Network {
  getSVG() {
    return this.svg;
  }

  constructor({container, nodes, width, height, range}) {
    this.draw = SVG(container).size("100%", "100%").panZoom();
    // this.draw.flip('y');
    this.nodes = nodes;
    this.width = width;
    this.height = height;
    this.range = range;
    this.nodeClickListeners = [];
    this.nodeMouseOverListeners = [];
    this.mousedownListeners = [];
    this.mousemoveListeners = [];
    this.mouseupListeners = [];

    this.svg = this.draw.node;
    this.selectLayer = this.draw.group();
    this.haloLayer = this.draw.group();
    this.edgeLayer = this.draw.group();
    this.nodeLayer = this.draw.group();

    this.nodes.forEach(node => {
      node.circle2 = this.haloLayer.circle(NODE_CIRCLE_RADIUS * 2 + 2).center(node.x*2.0, node.y*2.0).fill('#1E90FF');
      node.circle = this.nodeLayer
        .circle(NODE_CIRCLE_RADIUS * 2)
        .center(node.x*2.0, node.y*2.0)
        .fill('#ffffff');//('#b7b7b7');
      var text = this.draw.text(String(node.id))
      text.move(node.x*2.0-5, node.y*2.0).font({ fill: '#f06', family: 'Inconsolata' , size: 8})
      node.circle.click(() => {
        this.nodeClickListeners.forEach(listener => listener(node))
      });
      node.circle.on('mouseover', function() {
        document.getElementById('node-id').value = node.id;
      })
    });

    processNeighbors({nodes, range});
    let state = 'normal';
    let isSeleting = false;
    let polylines = [];


    this.draw.mousedown((e) => {
      let {x, y} = originPosition({x: e.clientX, y: e.clientY, svg: this.svg});
      if (state === 'deleting') {
        isSeleting = true;
        let polyline = this.selectLayer.polyline([x, y]).fill('#ffccca').stroke({width: 0.5});
        polylines.push(polyline);
      } else if (state === 'm') {
        this.polylineM = this.selectLayer.polyline([x, y]).fill('#f1ceff').stroke({width: 0.5});
      } else {
        this.mousedownListeners.forEach(l => l({x, y}));
      }
    });

    this.draw.mousemove((e) => {
      let {x, y} = originPosition({x: e.clientX, y: e.clientY, svg: this.svg});
      if (state === 'normal') {
        this.mousemoveListeners.forEach(l => l({x, y}));
      } else if (state === 'deleting') {
        if (isSeleting) {
          let polyline = polylines[polylines.length - 1];
          polyline.plot(polyline.array().value.concat([[x, y]])).fill('#ffccca').stroke({width: 0.5});
        }
      } else if (state === 'm') {
        this.polylineM.plot(this.polylineM.array().value.concat([[x, y]])).fill('#f1ceff').stroke({width: 0.5});
      }

    });

    this.draw.mouseup((e) => {
      if (state === 'deleting') {
        if (isSeleting) {
          isSeleting = false;
          state = 'normal';
          this.draw.panZoom();

          for (let node of this.nodes) {
            let {x, y} = node;
            for (let polyline of polylines) {
              if (inPolygon([x*2.0, y*2.0], polyline.array().value)) {
                node.halo = this.haloLayer
                  .circle(4 * NODE_CIRCLE_RADIUS)
                  .center(x, y)
                  .fill('#ff6262')
              }
            }
          }
        }
        // } else if (state === 'a') {
        //   if (isSeleting) {
        //     isSeleting = false;
        //     state = 'normal';
        //     this.draw.panZoom();
        //   }
        // } else if (state === 'b') {
        //   if (isSeleting) {
        //     isSeleting = false;
        //     state = 'normal';
        //     this.draw.panZoom();
        //   }
      } else if (state === 'm') {
        state = 'normal';
        this.draw.panZoom();
      } else {
        this.mouseupListeners.forEach(l => l());
      }
    });

    Mousetrap.bind(['command+d', 'ctrl+shift+d'], () => {
      if (state === 'normal') {
        state = 'deleting';
        this.draw.panZoom(false)
      }
    });
    // Mousetrap.bind(['command+a', 'ctrl+shift+a', 'ctrl+alt+a'], () => {
    //   if (state === 'normal') {
    //     state = 'a';
    //     this.draw.panZoom(false)
    //   }
    // });
    // Mousetrap.bind(['command+b', 'ctrl+shift+b', 'ctrl+alt+b'], () => {
    //   if (state === 'normal') {
    //     state = 'b';
    //     this.draw.panZoom(false)
    //   }
    // });

    Mousetrap.bind(['command+k', 'ctrl+shift+k'], () => {
      if (state === 'normal') {
        state = 'm';
        this.draw.panZoom(false)
      }
    });

    Mousetrap.bind(['enter', 'ctrl+enter'], () => {
      let points = this.polylineM.array().value;
      let minX = points.reduce((acc, val) => {
        return Math.min(acc, val[0])
      }, 1e9);
      let maxX = points.reduce((acc, val) => {
        return Math.max(acc, val[0])
      }, -1e9);
      let minY = points.reduce((acc, val) => {
        return Math.min(acc, val[1])
      }, 1e9);
      let maxY = points.reduce((acc, val) => {
        return Math.max(acc, val[1])
      }, -1e9);

      let num = parseInt(prompt("Number of node added?"));
      let currentId = this.nodes.reduce((acc, val) => Math.max(acc, val.id), 0);

      for (let i = 0; i < num; i++) {
        const x = Math.random() * (maxX - minX) + minX;
        const y = Math.random() * (maxY - minY) + minY;
        if (inPolygon([x, y], points)) {
          currentId++;
          let node = {
            x, y, id: currentId,
          };
          node.circle = this.nodeLayer
            .circle(NODE_CIRCLE_RADIUS * 2)
            .center(node.x*2.0, node.y*2.0)
            .fill('#b7b7b7');
          node.circle.click(() => {
            console.log(node);
            this.nodeClickListeners.forEach(listener => listener(node))
          });

          this.nodes.push(node);

        }
      }

      console.log(this.nodes.length)

      // this.nodeLayer
      //   .circle(NODE_CIRCLE_RADIUS * 8)
      //   .center(minX, minY)
      //   .fill('#000');
      // this.nodeLayer
      //   .circle(NODE_CIRCLE_RADIUS * 8)
      //   .center(maxX, maxY)
      //   .fill('#000');
    });

    Mousetrap.bind(['del', 'backspace'], () => {
      this.nodes
        .filter(node => node.halo)
        .forEach(node => {
          node.halo.remove();
          node.circle.remove();
          node.circle2.remove();
        });
      this.nodes = this.nodes
        .filter(node => !node.halo)
        .map((node, i) => {
          node.id = i;
          return node;
        });
      this.nodes.forEach(node => {
        node.circle.off('click');
        node.circle.click(() => {
          console.log(node);
          this.nodeClickListeners.forEach(listener => listener(node))
        })
      });
      polylines.forEach(p => p.remove());
    });

    Mousetrap.bind('esc', () => {
      state = 'normal';
      this.nodes
        .filter(node => node.halo)
        .forEach(node => {
          node.halo.remove();
          delete node.halo;
        });
      polylines.forEach(p => p.remove());
      polylines = [];
      if (this.polylineA) {
        this.polylineA.remove();
        this.polylineA = null;
      }
      if (this.polylineB) {
        this.polylineB.remove();
        this.polylineB = null;
      }
      if (this.polylineM) {
        this.polylineM.remove();
        this.polylineM = null;
      }
    });

    // this.indicatorLayer = this.draw.group().move(width + 20, 20);
    this.currentY = 40;

  }

  deleteHole(points) {
    let polylines = [];
    let bool = 0;
    points.forEach(point => {
      let {x, y} = {x: point[0], y: point[1]};
      if (bool == 0) {
        let polyline = this.selectLayer.polyline([x, y]).fill('#ffccca').stroke({width: 0.5});
        polylines.push(polyline);
        bool = 1;
      } else {
        let polyline = polylines[polylines.length - 1];
        polyline.plot(polyline.array().value.concat([[x, y]])).fill('#ffccca').stroke({width: 0.5});
      }
    });

    for (let node of this.nodes) {
      let {x, y} = node;
      for (let polyline of polylines) {
        if (inPolygon([x, y], polyline.array().value)) {
          node.halo = this.haloLayer
            .circle(4 * NODE_CIRCLE_RADIUS)
            .center(x, y)
            .fill('#ff6262')
        }
      }
    }

    this.nodes
      .filter(node => node.halo)
      .forEach(node => {
        node.halo.remove();
        node.circle.remove();
        node.circle2.remove();
      });
    this.nodes = this.nodes
      .filter(node => !node.halo)
      .map((node, i) => {
        node.id = i;
        return node;
      });
    this.nodes.forEach(node => {
      node.circle.off('click');
      node.circle.click(() => {
        console.log(node);
        this.nodeClickListeners.forEach(listener => listener(node))
      })
    });
    polylines.forEach(p => p.remove());
  }

  originPosition({x, y}) {
    let pt = this.svg.createSVGPoint();
    pt.x = x;
    pt.y = y;
    let svgP = pt.matrixTransform(this.svg.getScreenCTM().inverse());
    return {
      x: svgP.x, y: svgP.y
    }
  };

  remove() {
    this.draw.remove();
  }

  // addIndicator({color, name}) {
  //   let currentY = this.currentY;
  //   this.indicatorLayer.line(0, currentY, 100, currentY).stroke({width: 0.5, color});
  //   this.indicatorLayer.text(name).font({family: "Menlo"}).move(110, currentY - 8);
  //   this.currentY += 20;
  // };


  addNodeClickListener(listener) {
    this.nodeClickListeners.push(listener);
  }

  removeNodeClickListener(listener) {
    this.nodeClickListeners = this.nodeClickListeners.filter(l => l !== listener);
  }

  addNodeMouseOverListener(listener) {
    this.nodeMouseOverListeners.push(listener);
  }

  removeNodeMouseOverListener(listener) {
    this.nodeMouseOverListeners = this.nodeMouseOverListeners.filter(l => l !== listener);
  }

  addMousedownListener(listener) {
    this.mousedownListeners.push(listener);
  }

  removeMousedownListener(listener) {
    this.mousedownListeners = this.mousedownListeners.filter(l => l !== listener);
  }

  addMousemoveListener(listener) {
    this.mousemoveListeners.push(listener);
  }

  removeMousemoveListener(listener) {
    this.mousemoveListeners = this.mousemoveListeners.filter(l => l !== listener);
  }


  addMouseupListener(listener) {
    this.mouseupListeners.push(listener);
  }

  removeMouseupListener(listener) {
    this.mouseupListeners = this.mouseupListeners.filter(l => l !== listener);
  }

  drawLine({x1, y1, x2, y2, style}) {
    return this.edgeLayer.line(x1, y1, x2, y2).stroke(style);
  }

  drawText({text, x, y}) {
    return this.draw.text(text).move(x, y);
  }

  drawCircle({centerX, centerY, radius, style}) {
    return this.haloLayer
      .circle(radius * 2)
      .center(centerX, centerY)
      .fill('none')
      .stroke(style);
  }

  drawPoint({x, y, color}) {
    return this.nodeLayer
      .circle(NODE_CIRCLE_RADIUS * 2 + 2)
      .center(x, y)
      .fill(color);
  }

  drawHalo({x, y, radius, color}) {
    return this.haloLayer
      .circle(radius)
      .center(x, y)
      .fill(color);
  }

  getA() {
    if (!this.polylineA) {
      return this.nodes;
    }

    return this.nodes.filter(node => {
      return inPolygon([node.x*2.0, node.y*2.0], this.polylineA.array().value)
    })
  }

  getB() {
    if (!this.polylineB) {
      return this.nodes;
    }

    return this.nodes.filter(node => {
      return inPolygon([node.x*2.0, node.y*2.0], this.polylineB.array().value)
    })
  }

  drawArc({from, to, radius, style}) {
    return this.edgeLayer.path(describeArc(radius, from, to))
      .fill('none')
      .stroke(style)
  }


  drawArrow({from, to, style}) {
    console.log("draw arrow ne");
    let line1 = this.edgeLayer.line(from.x, from.y, to.x, to.y).stroke(style);

    let arrowLength = 12;
    let v = {x: from.x - to.x, y: from.y - to.y};
    let up = rotateVectorCCW({x: v.x, y: v.y, angle: Math.PI / 6});
    let upNormalized = {
      x: up.x / length(up) * arrowLength,
      y: up.y / length(up) * arrowLength,
    };

    let down = rotateVectorCCW({x: v.x, y: v.y, angle: -Math.PI / 6});
    let downNormalized = {
      x: down.x / length(down) * arrowLength,
      y: down.y / length(down) * arrowLength,
    };

    let o1 = {x: to.x + upNormalized.x, y: to.y + upNormalized.y};
    let o2 = {x: to.x + downNormalized.x, y: to.y + downNormalized.y};

    let line2 = this.edgeLayer.line(to.x, to.y, o1.x, o1.y).stroke(style);
    let line3 = this.edgeLayer.line(to.x, to.y, o2.x, o2.y).stroke(style);

    return [line1, line2, line3]
  }
}

function describeArc(radius, start, end) {
  let d = [
    "M", start.x, start.y,
    "A", radius, radius, 0, 0, 0, end.x, end.y
  ].join(" ");

  return d;
}


export default Network;
