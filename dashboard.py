# dashboard.py
import streamlit as st
import json
import os
import networkx as nx
import plotly.graph_objects as go

DATA_DIR = "dashboard_data"
GRAPH_FILE = os.path.join(DATA_DIR, "habit_graph.json")

st.set_page_config(layout="wide")
st.title("🧠 Habit Tracker Dashboard")

# ensure folder exists
if not os.path.exists(DATA_DIR):
    st.warning(f"No data folder found at {DATA_DIR}. Run the C++ app once to create exports.")
    st.stop()

# LOAD GRAPH
if os.path.exists(GRAPH_FILE):
    with open(GRAPH_FILE, 'r') as f:
        graph_data = json.load(f)
else:
    graph_data = {"nodes": [], "edges": []}

# Construct networkx graph
G = nx.DiGraph()
for n in graph_data.get("nodes", []):
    G.add_node(n)
for e in graph_data.get("edges", []):
    G.add_edge(e["source"], e["target"], weight=e.get("weight", 1))

# Graph viz
st.subheader("Habit Influence Graph")
if len(G) == 0:
    st.write("No graph data yet.")
else:
    pos = nx.spring_layout(G, seed=42)
    edge_x = []
    edge_y = []
    for src, dst in G.edges():
        x0, y0 = pos[src]
        x1, y1 = pos[dst]
        edge_x += [x0, x1, None]
        edge_y += [y0, y1, None]

    node_x = [pos[n][0] for n in G.nodes()]
    node_y = [pos[n][1] for n in G.nodes()]

    fig = go.Figure()
    fig.add_trace(go.Scatter(x=edge_x, y=edge_y, mode='lines', hoverinfo='none', line=dict(width=1)))
    fig.add_trace(go.Scatter(x=node_x, y=node_y, mode='markers+text',
                             text=list(G.nodes()), textposition="bottom center",
                             marker=dict(size=30)))
    st.plotly_chart(fig, use_container_width=True)

# Show each habit tree (textual or expandable)
st.subheader("Habit Progress Trees")
files = sorted([f for f in os.listdir(DATA_DIR) if f.endswith("_tree.json")])
if not files:
    st.write("No habit tree files found.")
else:
    for fname in files:
        path = os.path.join(DATA_DIR, fname)
        with open(path, 'r') as f:
            try:
                tree = json.load(f)
            except Exception:
                st.write(f"Could not parse {fname}")
                continue
        habit_name = fname.replace("_tree.json", "").replace("_", " ")
        with st.expander(habit_name, expanded=False):
            # Pretty print tree as indented text
            def draw(node, level=0):
                if not node:
                    return ""
                s = "  " * level + f"Day {node.get('day')} - {'✅' if node.get('success') else '❌'} (Mot: {node.get('motivation')})\n"
                left = node.get('left')
                right = node.get('right')
                s += draw(left, level+1)
                s += draw(right, level+1)
                return s
            st.text(draw(tree))

