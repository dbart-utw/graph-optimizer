import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib import gridspec
import math
import numpy as np


def is_number(x):
    return isinstance(x, (int, float)) and not isinstance(x, bool)


def parse_impls(preds):
    """Return list of (name, value_or_None, missing_flag)"""
    impls = []
    for n, v in preds.items():
        if is_number(v):
            impls.append((n, v, False))
        elif isinstance(v, str) and "No analytical model available" in v:
            impls.append((n, None, True))
    return impls


def plot_annotated_dag(data,title,target="runtime",max_cols=10,bar_width=0.6,compare_data=None):
    # Collect global implementation names (so color mapping is consistent)
    impl_names = set()
    all_positive_values = []

    def collect_impls(dataset):
        for bgo in dataset:
            perf = (bgo.get("performances") or [])
            prediction = perf[0].get(target, {}) if perf else {}
            for dev in ("CPU", "GPU"):
                for name, val in prediction.get(dev, {}).items():
                    if is_number(val):
                        impl_names.add(name)
                        if val > 0:
                            all_positive_values.append(val)
                    elif isinstance(val, str) and "No analytical model available" in val:
                        impl_names.add(name)

    collect_impls(data)
    if compare_data is not None:
        collect_impls(compare_data)

    impl_names = sorted(impl_names)

    # Color map (consistent across all subplots)
    cmap = plt.get_cmap("tab20")
    color_map = {name: cmap(i % 20) for i, name in enumerate(impl_names)}

    n_bgo = len(data)
    ncols = min(max_cols, n_bgo) if n_bgo > 0 else 1
    nrows = math.ceil(n_bgo / ncols)

    fig = plt.figure(figsize=(4 * ncols + 2, 3 * nrows + 1))
    outer_gs = gridspec.GridSpec(nrows, ncols, wspace=0.0, hspace=0.6)

    legend_handles = {}

    # compute a global y-range for log scale if possible
    use_log_scale_global = len(all_positive_values) > 0
    if use_log_scale_global:
        global_min = min(all_positive_values)
        global_max = max(all_positive_values)
        ymin = max(global_min * 0.1, 1e-12)
        ymax = global_max * 1.05
    else:
        ymin, ymax = None, None

    hatch_patterns = [None, "//"]  # original, compare

    for idx, bgo in enumerate(data):
        row = idx // ncols
        col = idx % ncols
        outer_cell = outer_gs[row, col]
        inner_gs = gridspec.GridSpecFromSubplotSpec(1, 2, subplot_spec=outer_cell, wspace=0.0)

        ax_cpu = fig.add_subplot(inner_gs[0])
        ax_gpu = fig.add_subplot(inner_gs[1])

        # Get BGO name
        bgo_name = bgo.get("name", f"id_{bgo.get('id')}")

        # Get data for both datasets
        perf = (bgo.get("performances") or [])
        prediction = perf[0].get(target, {}) if perf else {}
        comp_prediction = None
        comp_impls = None

        if compare_data:
            # Find matching BGO by name in compare_data
            match = next((b for b in compare_data if b.get("name") == bgo_name), None)
            if match:
                perf_c = (match.get("performances") or [])
                comp_prediction = perf_c[0].get(target, {}) if perf_c else {}

        for ax, dev in ((ax_cpu, "CPU"), (ax_gpu, "GPU")):
            # ----- replace the inner per-device plotting block with this -----
            preds = prediction.get(dev, {})
            impls = parse_impls(preds)
            if comp_prediction:
                comp_impls = parse_impls(comp_prediction.get(dev, {}))  # already computed earlier as comp_impls

            # Build name->(value, missing) maps for original and compare (value=None if missing)
            orig_map = {name: (val, missing) for name, val, missing in impls}
            comp_map = {name: (val, missing) for name, val, missing in comp_impls} if comp_impls else {}

            # Union of all implementation names for alignment (preserve a stable order)
            all_names = sorted(set(list(orig_map.keys()) + list(comp_map.keys())))

            # x positions for each implementation
            x = np.arange(len(all_names))

            # bar geometry
            if compare_data is None:
                width = bar_width
                offsets = [0.0]   # only one dataset
            else:
                width = bar_width * 0.45   # two bars side-by-side
                offsets = [-width/1.5, width/1.5]  # original left, compare right

            # Prepare arrays for original and compare values (None if missing)
            orig_vals = []
            orig_missing = []
            for n in all_names:
                v, missing = orig_map.get(n, (None, True))   # if not present => treat as missing
                orig_vals.append(v)
                orig_missing.append(missing)

            comp_vals = []
            comp_missing = []
            if compare_data is not None:
                for n in all_names:
                    v, missing = comp_map.get(n, (None, True))
                    comp_vals.append(v)
                    comp_missing.append(missing)

            # Draw original bars (only where numeric)
            # Bars for missing values are skipped (we don't draw zero-height bars, to keep log-scale safe)
            orig_bar_x = [xi + offsets[0] for xi in x]
            orig_bar_heights = [v if (v is not None and not isinstance(v, bool)) else 0 for v in orig_vals]
            # draw all as bars (zero height for missing) but avoid log problems by skipping log if necessary;
            # since we compute global ymin/ymax earlier, zero heights are fine visually because we use no bar for missing
            bars_orig = ax.bar(orig_bar_x, orig_bar_heights, width=width, align="center",
                            color=[color_map[n] for n in all_names],
                            hatch=hatch_patterns[0])

            # Draw compare bars if requested
            if compare_data is not None:
                comp_bar_x = [xi + offsets[1] for xi in x]
                comp_bar_heights = [v if (v is not None and not isinstance(v, bool)) else 0 for v in comp_vals]
                bars_comp = ax.bar(comp_bar_x, comp_bar_heights, width=width, align="center",
                                color=[color_map[n] for n in all_names],
                                hatch=hatch_patterns[1])

            # Legend handles: register only one handle per implementation (from original bars)
            for name, bar in zip(all_names, bars_orig):
                if name not in legend_handles:
                    legend_handles[name] = bar

            # Tick labels: color red if original dataset marks it "missing" (you can change policy)
            ax.set_xticks(x)
            ticks = ax.set_xticklabels(all_names, rotation=45, ha="right", fontsize=8)
            for tick, missing in zip(ticks, orig_missing):
                tick.set_color("red" if missing else "black")

            # Titles / y-axis as before
            ax.set_title(dev, fontsize=9, pad=2)
            if col == 0 and dev == "CPU":
                ax.set_ylabel("Time (ns)" if target == "runtime" else "Energy (Joules)")
            else:
                ax.yaxis.set_visible(False)

            # apply shared log scale and limits if needed
            if use_log_scale_global:
                ax.set_yscale("log")
                ax.set_ylim(ymin, ymax)
            ax.margins(y=0.05)


        # Add centered BGO title above CPU/GPU subplots
        bbox_cpu = ax_cpu.get_position()
        bbox_gpu = ax_gpu.get_position()
        center_x = (bbox_cpu.x0 + bbox_gpu.x1) / 2
        top_y = max(bbox_cpu.y1, bbox_gpu.y1)
        fig.text(center_x, top_y + 0.03, bgo_name, ha="center", va="bottom", fontsize=11, fontweight="bold")

    # Clean up unused cells
    total_cells = nrows * ncols
    for empty_idx in range(n_bgo, total_cells):
        r = empty_idx // ncols
        c = empty_idx % ncols
        ax = fig.add_subplot(outer_gs[r, c])
        ax.axis("off")

    fig.suptitle(title, fontsize=14, y=1.01)

    # Legend entries for dataset comparison
    baseline_patch = mpatches.Patch(
        facecolor="white",
        edgecolor="black",
        label="Prediction",
        hatch=None
    )
    compare_patch = mpatches.Patch(
        facecolor="white",
        edgecolor="black",
        hatch="//",
        label="Benchmark"
    )

    if compare_data is not None:
        fig.legend(
            handles=[baseline_patch, compare_patch],
            loc="upper right",
            bbox_to_anchor=(0.9, 1.02),
            borderaxespad=0.0,
            frameon=False,
            ncol = 2,
            fontsize=9,
            title_fontsize=10,
            alignment="center"
        )
    plt.show()


def sampling_prediction_validation(sampled_graph_benchmarks, full_graph_benchmarks, sampling_rate, target):
    dampening_factor = 0.5
    for i, bgo in enumerate(sampled_graph_benchmarks):
        for j, host in enumerate(bgo["performances"]):
            for impl, val in host[target]["CPU"].items():
                sampled_graph_benchmarks[i]["performances"][j]["runtime"]["CPU"][impl] = val / sampling_rate * dampening_factor
            for impl, val in host[target]["GPU"].items():
                sampled_graph_benchmarks[i]["performances"][j]["runtime"]["GPU"][impl] = val / sampling_rate * dampening_factor
    
    plot_annotated_dag(sampled_graph_benchmarks, "Sampling prediction validation", target, compare_data=full_graph_benchmarks)

def analytical_prediction_validation(analytical_prediction, full_graph_benchmarks, target):
    plot_annotated_dag(analytical_prediction, "Analytical prediction validation", target, compare_data=full_graph_benchmarks)



if __name__=='__main__':
    sampled_data = [{'name': 'pr', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'pr_gap': 172558695.0}, 'GPU': {'vertex_pull_warp': 2600862237.0, 'vertex_pull_warp_nodiv': 2251557357.0, 'rev_edgelist': 1658379635.0, 'vertex_push_warp': 1999473550.0, 'vertex_push': 1994066751.0, 'struct_edgelist': 1423946906.0, 'rev_struct_edgelist': 1520749261.0, 'edgelist': 1107574685.0, 'vertex_pull_nodiv': 1348917426.0, 'vertex_pull': 2268184334.0}}, 'energy': {'CPU': {'pr_gap': 4377928.0}, 'GPU': {'vertex_pull_warp': 79225777.0, 'vertex_pull_warp_nodiv': 81140697.0, 'rev_edgelist': 49424226.0, 'vertex_push_warp': 58265432.0, 'vertex_push': 58236824.0, 'struct_edgelist': 43875163.0, 'rev_struct_edgelist': 64690263.0, 'edgelist': 47620697.0, 'vertex_pull_nodiv': 52416504.0, 'vertex_pull': 90045126.0}}}]}, {'name': 'find_max', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'find_max_gb': 25830894.0, 'find_max_ca': 33403776.0}, 'GPU': {'find_max_gpu': 1175694420.0}}, 'energy': {'CPU': {'find_max_gb': 337675.0, 'find_max_ca': 922668.0}, 'GPU': {'find_max_gpu': 41498590.0}}}]}, {'name': 'bfs', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'bfs_gap': 83738854.0}, 'GPU': {}}, 'energy': {'CPU': {'bfs_gap': 1646464.0}, 'GPU': {}}}]}, {'name': 'find_path', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'fp_gb': 40191580.0}, 'GPU': {}}, 'energy': {'CPU': {'fp_gb': 1034766.0}, 'GPU': {}}}]}]
    data = [{'name': 'pr', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'pr_gap': 756548740.0, 'pr_gb': 321452867474.0}, 'GPU': {'vertex_pull_warp': 10296309332.0, 'vertex_pull_warp_nodiv': 8262262050.0, 'rev_edgelist': 3430216936.0, 'vertex_push_warp': 5317889293.0, 'vertex_push': 5794870768.0, 'struct_edgelist': 2565896186.0, 'rev_struct_edgelist': 3356835122.0, 'edgelist': 2449046458.0, 'vertex_pull_nodiv': 3296946960.0, 'vertex_pull': 13730383698.0}}, 'energy': {'CPU': {'pr_gap': 23618448.0, 'pr_gb': 11957584811.0}, 'GPU': {'vertex_pull_warp': 350879713.0, 'vertex_pull_warp_nodiv': 280579121.0, 'rev_edgelist': 116829567.0, 'vertex_push_warp': 184901250.0, 'vertex_push': 194543142.0, 'struct_edgelist': 88045722.0, 'rev_struct_edgelist': 114194048.0, 'edgelist': 86760334.0, 'vertex_pull_nodiv': 107133960.0, 'vertex_pull': 470917250.0}}}]}, {'name': 'find_max', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'find_max_gb': 62383886.0}, 'GPU': {'find_max_gpu': 1202181150.0}}, 'energy': {'CPU': {'find_max_gb': 1102199.0}, 'GPU': {'find_max_gpu': 39467256.0}}}]}, {'name': 'bfs', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'bfs_gap': 43596106.0, 'bfs_lagr': 284531522.0}, 'GPU': {}}, 'energy': {'CPU': {'bfs_gap': 522253.0, 'bfs_lagr': 10038520.0}, 'GPU': {}}}]}, {'name': 'find_path', 'performances': [{'host': 'H01', 'runtime': {'CPU': {'fp_gb': 67570501.0}, 'GPU': {}}, 'energy': {'CPU': {'fp_gb': 1343559.0}, 'GPU': {}}}]}]

    # sampling_prediction_validation(sampled_data, data, 0.1, "runtime")
    plot_annotated_dag(sampled_data, "Sampling prediction validation", "runtime")
    analytical_prediction_validation(sampled_data, data, "runtime")
