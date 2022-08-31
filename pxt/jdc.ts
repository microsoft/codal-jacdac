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
        return 0 // nothing yet
    }
}
