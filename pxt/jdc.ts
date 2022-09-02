namespace jdc {
    /**
     * Start jacdac-c stack
     */
    //% shim=jdc::start
    export function start(): void {
        return // do nothin' in sim
    }

    /**
     * Deploy a Jacdac-VM (Jacscript) program.
     */
    //% shim=jdc::deploy
    export function deploy(jacsprog: Buffer): number {
        control.simmessages.send("jacscript", jacsprog)
        // report errors via events?
        return 0
    }
}
