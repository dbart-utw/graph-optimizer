# Graph-Optimizer Beta Testing
This document describes the steps needed to be taken by the beta testers to test the functionality and usability of the Graph-Optimizer tool.


## Tool description
The Graph-Optimizer tool performs the following functions:
- Predicts the execution time (in milliseconds) and energy consumption (in Joules) for a given BGO or DAG of BGOs on a specific hardware configuration.
- Returns the model in symbolical form with graph properties as symbols or predicts execution times if the graph properties are specified.
- This is done via an API where issuing a POST request to `<api_url>/models` with the BGO DAG and hardware configuration returns an annotated DAG with calibrated symbolical models. Calling `<api_url>/evaluate` with the BGO DAG, hardware configuration, and graph properties returns an annotated DAG with predicted execution times.

Examples of the input DAG of BGOs and corresponding output annotated DAG are given below.

### Example input DAG
```JSON
[
    {
        "id": 0,
        "name": "bfs",
        "dependencies": []
    }
]
```

### Example output annotated DAG
1. With calibrated symbolical models:
```JSON
[
    {
        "id": 0,
        "name": "bfs",
        "dependencies": [],
        "performances": [
            {
                "host": "host1",
                "runtime": "n*2*1.521484375+n*(25.7+n*5.44296875)+n*(144.76)",
                "energy": "100n"
            },
            {
                "host": "host2",
                "runtime": "n*2*2.734375+n*(33.7+n*8.86875)+n*(186.76)",
                "energy": "100n"
            }
        ]
    }
]
```

2. With predicted execution times:
```JSON
[
    {
        "id": 0,
        "name": "bfs",
        "dependencies": [],
        "performances": [
            {
                "host": "host1",
                "runtime": 2180657559.375,
                "energy": 10020000
            },
            {
                "host": "host2",
                "runtime": 3552018575.0,
                "energy": 10020000
            }
        ]
    }
]
```

## Requirements
To use the Graph-Optimizer tool, ensure you have the following:

1. **Python**:
    - Version: 3.8 or higher

2. **Pip**:
    - Ensure Pip, the Python package installer, is installed and up to date.
    - Command to upgrade Pip (if needed):
      ```bash
      python -m pip install --upgrade pip
      ```

3. **Flask**:
    - Version: 3.0 or higher
    - Command to install Flask:
      ```bash
      pip install Flask
      ```

3. **Jupyter Notebook** (optional):
    - A jupyter notebook example is provided, showing how to issue a request to the API in python.
    - To run this example, make sure a working Jupyter Notebook environment is available.

## Testing Steps
### Step 1: Check if model exists
Ensure that the models you want to use are located in the `models` folder. The tool only supports models that are present in this directory. Only use BGO names that have their own subdirectory with energy and performance models in the `models` folder.

The structure of the `models` folder is as follows:

    models
    ├── <bgo1_name>
    │   ├── energy_model.py          # Energy model of BGO 1
    │   ├── performance_model.py     # Performance model of BGO 1
    │   └── ...                      # Miscellaneous files related to BGO 1
    ├── <bgo2_name>
    │   ├── energy_model.py          # Energy model of BGO 2
    │   ├── performance_model.py     # Performance model of BGO 2
    │   └── ...                      # Miscellaneous files related to BGO 2
    └── ...                 # Other BGOs and miscellaneous files

### Step 2: Specify input dag
Define the input BGO DAG in JSON format. This DAG should include one or multiple BGOs and their dependencies. The BGO name should match the name of the BGO folder in the `models` directory. The dependencies should be specified as a list of BGO id's that the current BGO depends on. For instance, consider the following example with multiple BGOs and dependencies:
```JSON
[
    {
        "id": 0,
        "name": "bc",
        "dependencies": []
    },
    {
        "id": 1,
        "name": "find_max",
        "dependencies": [0]
    },
    {
        "id": 2,
        "name": "bfs",
        "dependencies": [1]
    }
]
```


### Step 3: Specify hardware configuration
Provide the hardware configuration in JSON format. This configuration should list all unique available hosts in the data center, including details about CPUs and, if applicable, GPUs. An example is given below:
```JSON
{
    "hosts": [
        {
            "id": "H0",
            "name": "host1",
            "cpus": {
                "id": 1,
                "name": "intel xeon",
                "clock_speed": 2.10,
                "cores": 16,
                "benchmarks": {
                    "int_add": 2.4,
                    "float_mul": 10,
                    "q_pop": 11.2,
                    "DRAM_read": 62.5
                }
            },
            "gpus": [
                {
                    "id": 1,
                    "name": "RTX 3080",
                    "benchmarks": {
                        "int_add": 10,
                        "float_add": 15,
                        "L1_read": 5
                    }
                },
                {
                    "id": 2,
                    "name": "TitanX",
                    "benchmarks": {
                        "int_add": 10,
                        "float_add": 15
                    }
                }
            ]
        }
    ]
}
```
`Hosts` is a list of all unique available hosts in the data center. For example, a data center can have two types of nodes, one standard compute node, and a stronger node with multiple GPUs. Make sure all microbenchmarks required by the bgo model are present in the hardware configuration. This can be verified by checking the `performance_model.py` file in the respective BGO folder.
A more comprehensive example is provided in [api/hardware.json](api/hardware.json).

### Step 4: Specify graph properties
Create a single JSON file containing the graph properties. Ensure that all properties required by the BGO model are included.
```JSON
{
    "n": 20000,
    "m": 100000,
    "average_degree": 10,
    "directed": false,
    "weighted": true,
    "diameter": 5000,
    "clustering_coefficient": 0.6,
    "triangle_count": 50000
}
```

### Step 5: Run the prediction
1. Start the server using flask, by running the following command from the root directory:
    ```bash
    flask --app api/api.py run
    ```
    This will start the server on `localhost:5000`. Use the following command to start the server on a different port:
    ```bash
    flask --app api/api.py run --port <port_number>
    ```
2. Run the prediction by submitting a POST request to the api
    - For obtaining the calibrated symbolical models, issue a post request to `localhost:<port_number>/models`, with the following post data:
        - "hardware": The hardware configuration in JSON format, as a string.
        - "input_dag": The input BGO DAG in JSON format, as a string.
    - For obtaining the predicted execution times, issue a post request to `localhost:<port_number>/evaluate`, with the following post data:
        - "hardware": The hardware configuration in JSON format, as a string.
        - "input_dag": The input BGO DAG in JSON format, as a string.
        - "graph_properties": The graph properties in JSON format, as a string.
    - Examples on how to correctly call the API in python is given in [api/api_call.ipynb](api/api_call.ipynb).