/** @jsx MiniReact.createElement */
function Counter() {
    const [count, setCount] = MiniReact.useState(1);
    const [multiplier, setMultiplier] = MiniReact.useState(1);

    // Effect to log count changes
    MiniReact.useEffect(() => {
        console.log(`Count changed to: ${count}`);
        // Cleanup function
        return () => {
            console.log(`Cleaning up count: ${count}`);
        };
    }, [count]); // Only run when count changes

    // Effect to update document title
    MiniReact.useEffect(() => {
        document.title = `Count: ${count} × ${multiplier} = ${count * multiplier}`;
    }, [count, multiplier]); // Run when either count or multiplier changes

    // Effect with no dependencies - runs every render
    MiniReact.useEffect(() => {
        console.log('Component rendered');
    });

    return (
        <div className="counter">
            <h1>Count: {count} × {multiplier} = {count * multiplier}</h1>

            <div className="controls">
                <div className="count-controls">
                    <button className="button" onClick={() => setCount(c => c - 1)}>
                        Decrease
                    </button>
                    <button className="button" onClick={() => setCount(0)}>
                        Reset
                    </button>
                    <button className="button" onClick={() => setCount(c => c + 1)}>
                        Increase
                    </button>
                </div>

                <div className="multiplier-controls">
                    <h3>Multiplier: {multiplier}</h3>
                    <input
                        type="range"
                        min="1"
                        max="10"
                        value={multiplier}
                        onChange={(e) => setMultiplier(+e.target.value)}
                    />
                </div>
            </div>
        </div>
    );
}

const element = <Counter />;
const container = document.getElementById("root");
MiniReact.render(element, container);