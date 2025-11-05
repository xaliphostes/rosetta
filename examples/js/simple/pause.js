export async function pause(message = "Appuyez sur une touche pour continuer...") {
    return new Promise((resolve) => {
        process.stdout.write(message);

        // Passe en mode brut pour capter chaque touche sans attendre "Entrée"
        process.stdin.setRawMode(true);
        process.stdin.resume();
        process.stdin.setEncoding("utf8");

        process.stdin.once("data", (key) => {
            // Ctrl+C -> quitter proprement
            if (key === "\u0003") {
                process.stdout.write("\n");
                process.exit();
            }

            process.stdin.setRawMode(false);
            process.stdin.pause();
            process.stdout.write("\n");
            resolve();
        });
    });
}